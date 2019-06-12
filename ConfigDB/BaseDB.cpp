#include "BaseDB.h"
#include "NamedUnivType.h"
#include "RequireUpdateObj.h"
#include <cassert>

// 参照返しがエラーの際に返すダミー
UnivType BaseDB::m_dummy;

BaseDB::BaseDB()
{
	m_pRoot = this;
	m_pSubsetOwner = NULL;
}

BaseDB::~BaseDB()
{
	disconnectDB();
	clear();
}

CDB_ERR BaseDB::connectDB(BaseDB *pDB)
{
	assert(pDB != NULL);

	// 相手がツリーのトップでないと接続できない
	if(pDB->m_pRoot != pDB){
		return CDB_CONNECT_ERROR;
	}

	// 既に相手が接続されているかチェック
	if(m_pRoot->isConnected(pDB)){
		return CDB_ALRDY_CONNECTED;
	}

	// 既に相手に接続されているかチェック
	if(pDB->m_pRoot->isConnected(m_pRoot)){
		return CDB_ALRDY_CONNECTED;
	}

	// 名前付き変数の参照結合
	m_pRoot->connectNamedObjects(pDB->m_pRoot);
	pDB->m_pRoot->connectNamedObjects(m_pRoot);

	// 検索リストに追加
	m_searchList.push_back(pDB->m_pRoot);
	// 追加されたDBの検索トップを自身の検索トップに設定
	pDB->m_pRoot = m_pRoot;

	return CDB_OK;
}

CDB_ERR BaseDB::disconnectDB(BaseDB *pDB/* = NULL*/)
{
	if(pDB != NULL){
		if(m_searchList.isExist(pDB)){
			// 追加されたDBの検索トップをそのDB自身に設定
			pDB->m_pRoot = pDB;
			// 検索リストから削除
			m_searchList.erase(pDB);

			// 名前付き変数の参照結合
			m_pRoot->disconnectNamedObjects(pDB->m_pRoot);
			pDB->m_pRoot->disconnectNamedObjects(m_pRoot);
		}else{
			return CDB_CANNOT_DISCONNECT;
		}
	}else{
		// 全接続の解除
		int n = m_searchList.size();
		for(int i = 0; i < n; i++){
			pDB = m_searchList[i];
			// 追加されたDBの検索トップをそのDB自身に設定
			pDB->m_pRoot = pDB;
			// 検索リストから削除
			m_searchList.erase(pDB);

			// 名前付き変数の参照結合
			m_pRoot->disconnectNamedObjects(pDB->m_pRoot);
			pDB->m_pRoot->disconnectNamedObjects(m_pRoot);
		}
		m_searchList.clear();
	}
	return CDB_OK;
}

bool BaseDB::isConnected(BaseDB *pDB) const
{
	if(pDB == this) return true;

	int n = m_searchList.size();
	for(int i = 0; i < n; i++){
		if(m_searchList[i]->isConnected(pDB)) return true;
	}

	return false;
}

void BaseDB::connectNamedObjects(BaseDB *pDB)
{
	// 【接続先→自分】へのリンク作成
	createLink(pDB, this);

	// 【自分→接続先】へのリンク作成
	createLink(this, pDB);

	// 自身の検索リスト中に接続
	int n = m_searchList.size();
	for(int i = 0; i < n; i++){
		m_searchList[i]->connectNamedObjects(pDB);
	}
}

void BaseDB::disconnectNamedObjects(BaseDB *pDB)
{
	// 【接続先→自分】へのリンク解除
	deleteLink(pDB, this);

	// 【自分→接続先】へのリンク解除
	deleteLink(this, pDB);

	// 自身の検索リスト中から接続解除
	int n = m_searchList.size();
	for(int i = 0; i < n; i++){
		m_searchList[i]->disconnectNamedObjects(pDB);
	}
}

void BaseDB::createLink(BaseDB *pFrom, BaseDB *pTo)
{
	for(NamedUnivType *pUnivType = pFrom->m_map.getNext(true); pUnivType; pUnivType = pFrom->m_map.getNext(false)){
		if(!pUnivType->isValid()){
			try{
				UnivType* p = pTo->search(pUnivType->getName());
				// TODO : 参照方法を考える（現時点では、DB間接続はフル参照は使えないとしておく）
				pUnivType->setConstReference(p);	// 参照として接続する
			}
			catch(CdbException& e){
				if(e.getErrorCode() == CDB_NOTFOUND){
					continue;
				}else{
					assert(false);
					return;
				}
			}
		}
	}
}

void BaseDB::deleteLink(BaseDB *pFrom, BaseDB *pTo)
{
	for(NamedUnivType *pUnivType = pFrom->m_map.getNext(true); pUnivType; pUnivType = pFrom->m_map.getNext(false)){
		if(pUnivType->isReference()){
			try{
				(void)pTo->search(pUnivType->getName());
				// 見つかった場合、接続を解除して無効化する
				pUnivType->invalidate();
			}
			catch(CdbException& e){
				if(e.getErrorCode() == CDB_NOTFOUND){
					continue;
				}else{
					assert(false);
					return;
				}
			}
		}
	}
}

UnivType& BaseDB::addNew()
{
	UnivType *pUnivType = new NamedUnivType;
	if(pUnivType == NULL) throw CdbException(CDB_BAD_ALLOC);
	m_nonameList.push_back(pUnivType);
	return *pUnivType;
}

// キー名称の重複チェックは行わないことに注意
UnivType& BaseDB::addNew(const char *pName)
{
	NamedUnivType *pUnivType = new NamedUnivType;
	if(pUnivType == NULL) throw CdbException(CDB_BAD_ALLOC);
	pUnivType->setName(pName);
	m_map.add(pUnivType->getName(), pUnivType);
	return *pUnivType;
}

// キー名称の重複チェックは行わないことに注意
UnivType& BaseDB::addNew(const char *pName, CDB_ERR& error)
{
	error = CDB_OK;
	try{
		return addNew(pName);
	}
	catch(CdbException& e){
		error = e.getErrorCode();
		return m_dummy;
	}
}

// サブセットの新規作成（エラーを例外で返す）
BaseDB *BaseDB::createSubset(UnivType *pObj)
{
	BaseDB *pNewDB = new BaseDB;
	pNewDB->m_pSubsetOwner = this;	//　所有者を自身に設定
	pObj->setSubset(pNewDB);	// 登録、以降このオブジェクトの管理はこちらに移る
	return pNewDB;
}

// bSubsetOwnerSearch : 自身がサブセットである場合、所有者のDBまで検索を行うかどうか
UnivType *BaseDB::search(const char *pName, bool bSubsetOwnerSearch/* = true*/)
{
	// 自身と自身が持つDBリストを検索
	UnivType *pUnivType = m_pRoot->searchSelf(pName);
	if(pUnivType != NULL){
		return pUnivType;
	}

	// ルートから自身を除いて検索
	pUnivType = m_pRoot->searchSelf(pName, this);

	// 自身がサブセットである場合、親を検索
	if(bSubsetOwnerSearch && (m_pSubsetOwner != NULL)){
		pUnivType = m_pSubsetOwner->search(pName);
	}

	return pUnivType;
}

// pName : 検索名称
// pSearchExcept : 検索除外DB
UnivType *BaseDB::searchSelf(const char *pName, BaseDB *pSearchExcept/* = NULL*/)
{
	// 検索除外対象であった場合、何もしない
	if(pSearchExcept == this){
		return NULL;
	}

	// 自身のDBから検索
	UnivType *pUnivType = m_map.find(pName);
	if(pUnivType != NULL) return pUnivType;

	// 自身の検索リストから検索
	int n = m_searchList.size();
	for(int i = 0; i < n; i++){
		pUnivType = m_searchList[i]->searchSelf(pName, pSearchExcept);
		// 見つかったらそこで検索を打ち切る
		if(pUnivType != NULL) return pUnivType;
	}

	// 見つからない場合
	return NULL;
}

bool BaseDB::confirmAddEnable(const char *pName)
{
	// 自身がサブセットである場合、自身のみが同じ名前をもっていなければ可能
	if(m_pSubsetOwner != NULL){
		return m_map.find(pName) == NULL;
	}else{
		// 検索範囲内で見つからなければ可能
		return search(pName) == NULL;
	}
}

// UnivType型（定数属性でも取得できる、エラーを例外で返す）
UnivType& BaseDB::getUnsafeUnivType(const char *pName, bool bSubsetOwnerSearch/* = true*/)
{
	UnivType *pUnivType = search(pName, bSubsetOwnerSearch);
	if(pUnivType == NULL){
		throw CdbException(CDB_NOTFOUND, pName);
		return m_dummy;
	}
	return *pUnivType;
}

// UnivType型（エラーを例外で返す）
UnivType& BaseDB::getUnivType(const char *pName)
{
	UnivType *pUnivType = search(pName);
	if(pUnivType != NULL){
		// 定数オブジェクトの場合にはエラー
		if(pUnivType->isConstant()){
			throw CdbException(CDB_WRITE_TO_CONST1, pName);
		}
		return *pUnivType;
	}else{
		throw CdbException(CDB_NOTFOUND, pName);
		return m_dummy;
	}
}

const UnivType& BaseDB::getConstUnivType(const char *pName) const
{
	UnivType *pUnivType = const_cast<BaseDB *>(this)->search(pName);
	if(pUnivType != NULL){
		return *pUnivType;
	}else{
		throw CdbException(CDB_NOTFOUND, pName);
		return m_dummy;
	}
}

// UnivType型（エラーコードで返す）
UnivType& BaseDB::getUnivType(const char *pName, CDB_ERR& error)
{
	error = CDB_OK;
	try{
		return getUnivType(pName);
	}
	catch(CdbException& e){
		error = e.getErrorCode();
	}
	return m_dummy;
}

const UnivType& BaseDB::getConstUnivType(const char *pName, CDB_ERR& error) const
{
	error = CDB_OK;
	try{
		return getConstUnivType(pName);
	}
	catch(CdbException& e){
		error = e.getErrorCode();
	}
	return m_dummy;
}

// 整数値
CDB_ERR BaseDB::getInt(const char *pName, int& x)
{
	try{
		x = (int)getConstUnivType(pName);
		return CDB_OK;
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
}

CDB_ERR BaseDB::setInt(const char *pName, int x)
{
	try{
		getUnivType(pName) = x;
		return CDB_OK;
	}
	catch(CdbException& e){
		CDB_ERR error = e.getErrorCode();
		if(error == CDB_NOTFOUND){
			addNew(pName, error) = x;
		}
		return error;
	}
}

// 符号なし整数値
CDB_ERR BaseDB::getUInt(const char *pName, unsigned int& x)
{
	try{
		x = (unsigned int)getConstUnivType(pName);
		return CDB_OK;
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
}

CDB_ERR BaseDB::setUInt(const char *pName, unsigned int x)
{
	try{
		getUnivType(pName) = x;
		return CDB_OK;
	}
	catch(CdbException& e){
		CDB_ERR error = e.getErrorCode();
		if(error == CDB_NOTFOUND){
			addNew(pName, error) = x;
		}
		return error;
	}
}

// 実数値
CDB_ERR BaseDB::getReal(const char *pName, UnivType::Real& x)
{
	try{
		x = (UnivType::Real)getConstUnivType(pName);
		return CDB_OK;
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
}

CDB_ERR BaseDB::setReal(const char *pName, UnivType::Real x)
{
	try{
		getUnivType(pName) = x;
		return CDB_OK;
	}
	catch(CdbException& e){
		CDB_ERR error = e.getErrorCode();
		if(error == CDB_NOTFOUND){
			addNew(pName, error) = x;
		}
		return error;
	}
}

// ブール値
CDB_ERR BaseDB::getBool(const char *pName, bool& x)
{
	try{
		x = (bool)getConstUnivType(pName);
		return CDB_OK;
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
}

CDB_ERR BaseDB::setBool(const char *pName, bool x)
{
	try{
		getUnivType(pName) = x;
		return CDB_OK;
	}
	catch(CdbException& e){
		CDB_ERR error = e.getErrorCode();
		if(error == CDB_NOTFOUND){
			addNew(pName, error) = x;
		}
		return error;
	}
}

// ポインタ値
CDB_ERR BaseDB::getPtr(const char *pName, void*& p)
{
	try{
		const UnivType& obj = getConstUnivType(pName);
		p = obj.getPointer();
		return CDB_OK;
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
}

CDB_ERR BaseDB::setPtr(const char *pName, void* p)
{
	try{
		getUnivType(pName).setPointer(p);
		return CDB_OK;
	}
	catch(CdbException& e){
		CDB_ERR error = e.getErrorCode();
		if(error == CDB_NOTFOUND){
			UnivType& obj = addNew(pName, error);
			obj.setPointer(p);
		}
		return error;
	}
}

// 文字列
CDB_ERR BaseDB::getString(const char *pName, const char*& p)
{
	try{
		p = (const char *)getConstUnivType(pName);
		return CDB_OK;
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
}

CDB_ERR BaseDB::setString(const char *pName, const char* p)
{
	try{
		getUnivType(pName) = p;
		return CDB_OK;
	}
	catch(CdbException& e){
		CDB_ERR error = e.getErrorCode();
		if(error == CDB_NOTFOUND){
			addNew(pName, error) = p;
		}
		return error;
	}
}

// バイナリデータ
CDB_ERR BaseDB::getBinary(const char *pName, void*& p, size_t& size)
{
	try{
		const UnivType& obj = getConstUnivType(pName);
		p = obj.getBinary();
		size = obj.getBinarySize();
		return CDB_OK;
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
}

CDB_ERR BaseDB::setBinary(const char *pName, const void* p, size_t size)
{
	try{
		getUnivType(pName).setBinary(p, size);
		return CDB_OK;
	}
	catch(CdbException& e){
		CDB_ERR error = e.getErrorCode();
		if(error == CDB_NOTFOUND){
			UnivType& obj = addNew(pName, error);
			obj.setBinary(p, size);
		}
		return error;
	}
}

// 配列オブジェクト（過去バージョンとの互換性のため。利用は推奨されない。）
CDB_ERR BaseDB::getArrayObj(const char *pName, void*& p, size_t& size)
{
	// 未実装
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

CDB_ERR BaseDB::setArrayObj(const char *pName, const void* p, size_t size)
{
	// 未実装
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

// 要素の他データベースからのコピー
CDB_ERR BaseDB::copy(const char *pName, const BaseDB*  pSource)
{
	const UnivType *pUnivType = pSource->m_map.find(pName);
	if(pUnivType != NULL){
		try{
			getUnivType(pName).copy(*pUnivType);
			return CDB_OK;
		}
		catch(CdbException& e){
			CDB_ERR error = e.getErrorCode();
			if(error == CDB_NOTFOUND){
				addNew(pName, error).copy(*pUnivType);
			}
			return error;
		}
	}else{
		return CDB_NOTFOUND;
	}
}

// 全要素の他データベースからのコピー
CDB_ERR BaseDB::copy(const BaseDB*  pSource)
{
	clear();

	// 【注】m_pRootも同時にコピーしてしまうと検索開始地点が変わってしまうのでダメ
	m_pSubsetOwner = pSource->m_pSubsetOwner;

	WrappedVector<std::pair<UnivType *, UnivType *> > ReplaceList;

	try{
		const NamedUnivType *pUnivType;
		for(pUnivType = const_cast<BaseDB *>(pSource)->m_map.getNext(true); pUnivType != NULL; pUnivType = const_cast<BaseDB *>(pSource)->m_map.getNext()){
			UnivType& newobj = addNew(pUnivType->getName());
			newobj.copy(*pUnivType);
			ReplaceList.push_back(std::pair<UnivType *, UnivType *>(const_cast<NamedUnivType *>(pUnivType), &newobj));
		}
		// サブセット内での参照が外れているのを元に戻す
		for(int i = 0; i < ReplaceList.size(); i++){
			std::pair<UnivType *, UnivType *> element = ReplaceList.getAt(i);
			replaceReference(element.first, element.second);
		}
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
	return CDB_OK;
}

CDB_ERR BaseDB::evalCopy(const BaseDB*  pSource)
{
	clear();

	// 【注】m_pRootも同時にコピーしてしまうと検索開始地点が変わってしまうのでダメ
	m_pSubsetOwner = pSource->m_pSubsetOwner;

	try{
		const NamedUnivType *pUnivType;
		for(pUnivType = const_cast<BaseDB *>(pSource)->m_map.getNext(true); pUnivType != NULL; pUnivType = const_cast<BaseDB *>(pSource)->m_map.getNext()){
			addNew(pUnivType->getName()).evalCopy(*pUnivType);
		}
	}
	catch(CdbException& e){
		return e.getErrorCode();
	}
	return CDB_OK;
}

// 要素の他データベースからの移動
CDB_ERR BaseDB::move(const char *pName, BaseDB*  pSource)
{
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

CDB_ERR BaseDB::remove(const char *pName)
{
	// TODO : 削除の前に、そのobjを参照しているものを全てinvalidateしなくてはいけない
	// もしくは、参照されていたら削除できないようにする必要がある

	// 自身のDBから検索
	UnivType *pUnivType = m_map.find(pName);
	if(pUnivType != NULL){
		m_map.erase(pName);
		delete pUnivType;
	}else{
		return CDB_NOTFOUND;
	}

	return CDB_OK;
}

CDB_ERR BaseDB::rename(UnivType *pTarget, const char *pName)
{
	// 既に同名のオブジェクトが存在していたらエラーとする
	if(!confirmAddEnable(pName)){
		return CDB_KEY_COLLISION;
	}

	// オブジェクトに名前を設定
	NamedUnivType *pNamedTarget = dynamic_cast<NamedUnivType *>(pTarget);
	//NamedUnivType *pNamedTarget = reinterpret_cast<NamedUnivType *>(pTarget);
	pNamedTarget->setName(pName);

	if(m_nonameList.erase(pTarget)){
		// もともと名前なしオブジェクトであった場合
		m_map.add(pNamedTarget->getName(), pNamedTarget);
	}else{
		// もともと名前つきオブジェクトであった場合
		pNamedTarget->setName(pName);
	}

	return CDB_OK;
}

void BaseDB::issueNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	// ルート以下の全ての要素に更新通知する
	receiveNotifyUpdated(pUnivType->getReferenceRoot(true), kind);
}

bool BaseDB::receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	// ルート以下の全ての要素に更新通知する
	return m_pRoot->receiveNotifyUpdatedChildren(pUnivType, kind);
}

void BaseDB::update()
{
	// ルート以下の全ての要素を更新する
	m_pRoot->updateChildren();
}

bool BaseDB::receiveNotifyUpdatedChildren(const UnivType *pUnivType, enum UpdateKind kind)
{
	int i, n;
	bool updated = false;
	// 自身の更新リストを更新
	n = m_updateList.size();
	for(i = 0; i < n; i++){
		if(m_updateList[i]->receiveNotifyUpdated(pUnivType, kind)){
			updated = true;
		}
	}
	// 自身の検索リストを更新
	n = m_searchList.size();
	for(i = 0; i < n; i++){
		if(m_searchList[i]->receiveNotifyUpdatedChildren(pUnivType, kind)){
			updated = true;
		}
	}
	return updated;
}

// 参照先の置き換え
bool BaseDB::replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith)
{
	bool updated = false;
	// 120728 更新リスト中のオブジェクトは置き換えを行わない
	// replaceReferenceの用途である、サブセットのコピーにおいては、必要がないため
	// そもそも、更新リストが何のために存在しているのか不明(必要ないならば消していいかも)

	UnivType *pUnivType;
	for(pUnivType = const_cast<BaseDB *>(this)->m_map.getNext(true); pUnivType != NULL; pUnivType = const_cast<BaseDB *>(this)->m_map.getNext()){
		if(pUnivType->replaceReference(pFindWhat, pReplaceWith)){
			updated = true;
		}
	}
	return updated;
}

void BaseDB::updateChildren()
{
	int i, n;
	// 自身の更新リストを更新
	n = m_updateList.size();
	for(i = 0; i < n; i++){
		m_updateList[i]->update();
		n = m_updateList.size();
	}
	// 自身の検索リストを更新
	n = m_searchList.size();
	for(i = 0; i < n; i++){
		m_searchList[i]->updateChildren();
	}
}

// 全要素の消去
void BaseDB::clear()
{
#ifdef _DEBUG
	System::Console::WriteLine("");
	System::Console::WriteLine("[info] m_searchList="+m_searchList.size()+" m_updateList="+m_updateList.size(), " exists before baseDC::clear()");
	System::Console::WriteLine("------------------------------");
#endif

	UnivType *pUnivType;
	for(pUnivType = m_map.getNext(true); pUnivType != NULL; pUnivType = m_map.getNext()){
		delete pUnivType;
	}
	m_map.clear();

	int n = m_nonameList.size();
	for(int i = 0; i < n; i++){
		delete m_nonameList.getAt(i);
	}
	m_nonameList.clear();

#ifdef _DEBUG
	if(m_updateList.size() != 0){
		System::Console::WriteLine("[warning] m_searchList="+m_searchList.size()+" m_updateList="+m_updateList.size(), " remains");
	}
#endif

	m_searchList.clear();
	m_updateList.clear();
}

// キーの存在確認
// internal = falseのときには、検索リンク先の検索も行う
bool BaseDB::isExistKey(const char *pName, bool internal/* = false*/) const 
{
	const UnivType *pUnivType;
	if(internal){
		// 自身のDBから検索
		pUnivType = m_map.find(pName);
	}else{
		// 接続されている全体のDBから検索
		pUnivType = const_cast<BaseDB *>(this)->search(pName);
	}

	return pUnivType != NULL;
}

#ifdef DEBUG_PRINT
#define TAB_INDENT(nIndentLevel) { for(int n = 0; n < nIndentLevel; n++) System::Console::Write("\t"); }
void BaseDB::debugPrintStructure(int nIndentLevel) const
{
	System::Console::WriteLine("{");
	bool isFirst = true;
	const NamedUnivType *pUnivType;
	for(pUnivType = const_cast<BaseDB *>(this)->m_map.getNext(true); pUnivType != NULL; pUnivType = const_cast<BaseDB *>(this)->m_map.getNext()){
		if(isFirst){
			isFirst = false;
		}else{
			System::Console::WriteLine(",");
		}
		TAB_INDENT(nIndentLevel);
#if 0
		System::String^ Name = gcnew System::String(pUnivType->getName());
		System::Console::Write("["+Name+"]=");
#else
		pUnivType->debugPrintStructure(nIndentLevel);
		System::Console::Write("=");
#endif
		pUnivType->debugPrintStructure(nIndentLevel + 1, false);
	}
	System::Console::Write("}");
}
#endif
