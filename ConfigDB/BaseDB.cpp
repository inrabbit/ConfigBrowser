#include "BaseDB.h"
#include "NamedUnivType.h"
#include "RequireUpdateObj.h"
#include <cassert>

// �Q�ƕԂ����G���[�̍ۂɕԂ��_�~�[
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

	// ���肪�c���[�̃g�b�v�łȂ��Ɛڑ��ł��Ȃ�
	if(pDB->m_pRoot != pDB){
		return CDB_CONNECT_ERROR;
	}

	// ���ɑ��肪�ڑ�����Ă��邩�`�F�b�N
	if(m_pRoot->isConnected(pDB)){
		return CDB_ALRDY_CONNECTED;
	}

	// ���ɑ���ɐڑ�����Ă��邩�`�F�b�N
	if(pDB->m_pRoot->isConnected(m_pRoot)){
		return CDB_ALRDY_CONNECTED;
	}

	// ���O�t���ϐ��̎Q�ƌ���
	m_pRoot->connectNamedObjects(pDB->m_pRoot);
	pDB->m_pRoot->connectNamedObjects(m_pRoot);

	// �������X�g�ɒǉ�
	m_searchList.push_back(pDB->m_pRoot);
	// �ǉ����ꂽDB�̌����g�b�v�����g�̌����g�b�v�ɐݒ�
	pDB->m_pRoot = m_pRoot;

	return CDB_OK;
}

CDB_ERR BaseDB::disconnectDB(BaseDB *pDB/* = NULL*/)
{
	if(pDB != NULL){
		if(m_searchList.isExist(pDB)){
			// �ǉ����ꂽDB�̌����g�b�v������DB���g�ɐݒ�
			pDB->m_pRoot = pDB;
			// �������X�g����폜
			m_searchList.erase(pDB);

			// ���O�t���ϐ��̎Q�ƌ���
			m_pRoot->disconnectNamedObjects(pDB->m_pRoot);
			pDB->m_pRoot->disconnectNamedObjects(m_pRoot);
		}else{
			return CDB_CANNOT_DISCONNECT;
		}
	}else{
		// �S�ڑ��̉���
		int n = m_searchList.size();
		for(int i = 0; i < n; i++){
			pDB = m_searchList[i];
			// �ǉ����ꂽDB�̌����g�b�v������DB���g�ɐݒ�
			pDB->m_pRoot = pDB;
			// �������X�g����폜
			m_searchList.erase(pDB);

			// ���O�t���ϐ��̎Q�ƌ���
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
	// �y�ڑ��恨�����z�ւ̃����N�쐬
	createLink(pDB, this);

	// �y�������ڑ���z�ւ̃����N�쐬
	createLink(this, pDB);

	// ���g�̌������X�g���ɐڑ�
	int n = m_searchList.size();
	for(int i = 0; i < n; i++){
		m_searchList[i]->connectNamedObjects(pDB);
	}
}

void BaseDB::disconnectNamedObjects(BaseDB *pDB)
{
	// �y�ڑ��恨�����z�ւ̃����N����
	deleteLink(pDB, this);

	// �y�������ڑ���z�ւ̃����N����
	deleteLink(this, pDB);

	// ���g�̌������X�g������ڑ�����
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
				// TODO : �Q�ƕ��@���l����i�����_�ł́ADB�Ԑڑ��̓t���Q�Ƃ͎g���Ȃ��Ƃ��Ă����j
				pUnivType->setConstReference(p);	// �Q�ƂƂ��Đڑ�����
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
				// ���������ꍇ�A�ڑ����������Ė���������
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

// �L�[���̂̏d���`�F�b�N�͍s��Ȃ����Ƃɒ���
UnivType& BaseDB::addNew(const char *pName)
{
	NamedUnivType *pUnivType = new NamedUnivType;
	if(pUnivType == NULL) throw CdbException(CDB_BAD_ALLOC);
	pUnivType->setName(pName);
	m_map.add(pUnivType->getName(), pUnivType);
	return *pUnivType;
}

// �L�[���̂̏d���`�F�b�N�͍s��Ȃ����Ƃɒ���
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

// �T�u�Z�b�g�̐V�K�쐬�i�G���[���O�ŕԂ��j
BaseDB *BaseDB::createSubset(UnivType *pObj)
{
	BaseDB *pNewDB = new BaseDB;
	pNewDB->m_pSubsetOwner = this;	//�@���L�҂����g�ɐݒ�
	pObj->setSubset(pNewDB);	// �o�^�A�ȍ~���̃I�u�W�F�N�g�̊Ǘ��͂�����Ɉڂ�
	return pNewDB;
}

// bSubsetOwnerSearch : ���g���T�u�Z�b�g�ł���ꍇ�A���L�҂�DB�܂Ō������s�����ǂ���
UnivType *BaseDB::search(const char *pName, bool bSubsetOwnerSearch/* = true*/)
{
	// ���g�Ǝ��g������DB���X�g������
	UnivType *pUnivType = m_pRoot->searchSelf(pName);
	if(pUnivType != NULL){
		return pUnivType;
	}

	// ���[�g���玩�g�������Č���
	pUnivType = m_pRoot->searchSelf(pName, this);

	// ���g���T�u�Z�b�g�ł���ꍇ�A�e������
	if(bSubsetOwnerSearch && (m_pSubsetOwner != NULL)){
		pUnivType = m_pSubsetOwner->search(pName);
	}

	return pUnivType;
}

// pName : ��������
// pSearchExcept : �������ODB
UnivType *BaseDB::searchSelf(const char *pName, BaseDB *pSearchExcept/* = NULL*/)
{
	// �������O�Ώۂł������ꍇ�A�������Ȃ�
	if(pSearchExcept == this){
		return NULL;
	}

	// ���g��DB���猟��
	UnivType *pUnivType = m_map.find(pName);
	if(pUnivType != NULL) return pUnivType;

	// ���g�̌������X�g���猟��
	int n = m_searchList.size();
	for(int i = 0; i < n; i++){
		pUnivType = m_searchList[i]->searchSelf(pName, pSearchExcept);
		// ���������炻���Ō�����ł��؂�
		if(pUnivType != NULL) return pUnivType;
	}

	// ������Ȃ��ꍇ
	return NULL;
}

bool BaseDB::confirmAddEnable(const char *pName)
{
	// ���g���T�u�Z�b�g�ł���ꍇ�A���g�݂̂��������O�������Ă��Ȃ���Ή\
	if(m_pSubsetOwner != NULL){
		return m_map.find(pName) == NULL;
	}else{
		// �����͈͓��Ō�����Ȃ���Ή\
		return search(pName) == NULL;
	}
}

// UnivType�^�i�萔�����ł��擾�ł���A�G���[���O�ŕԂ��j
UnivType& BaseDB::getUnsafeUnivType(const char *pName, bool bSubsetOwnerSearch/* = true*/)
{
	UnivType *pUnivType = search(pName, bSubsetOwnerSearch);
	if(pUnivType == NULL){
		throw CdbException(CDB_NOTFOUND, pName);
		return m_dummy;
	}
	return *pUnivType;
}

// UnivType�^�i�G���[���O�ŕԂ��j
UnivType& BaseDB::getUnivType(const char *pName)
{
	UnivType *pUnivType = search(pName);
	if(pUnivType != NULL){
		// �萔�I�u�W�F�N�g�̏ꍇ�ɂ̓G���[
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

// UnivType�^�i�G���[�R�[�h�ŕԂ��j
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

// �����l
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

// �����Ȃ������l
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

// �����l
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

// �u�[���l
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

// �|�C���^�l
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

// ������
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

// �o�C�i���f�[�^
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

// �z��I�u�W�F�N�g�i�ߋ��o�[�W�����Ƃ̌݊����̂��߁B���p�͐�������Ȃ��B�j
CDB_ERR BaseDB::getArrayObj(const char *pName, void*& p, size_t& size)
{
	// ������
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

CDB_ERR BaseDB::setArrayObj(const char *pName, const void* p, size_t size)
{
	// ������
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

// �v�f�̑��f�[�^�x�[�X����̃R�s�[
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

// �S�v�f�̑��f�[�^�x�[�X����̃R�s�[
CDB_ERR BaseDB::copy(const BaseDB*  pSource)
{
	clear();

	// �y���zm_pRoot�������ɃR�s�[���Ă��܂��ƌ����J�n�n�_���ς���Ă��܂��̂Ń_��
	m_pSubsetOwner = pSource->m_pSubsetOwner;

	WrappedVector<std::pair<UnivType *, UnivType *> > ReplaceList;

	try{
		const NamedUnivType *pUnivType;
		for(pUnivType = const_cast<BaseDB *>(pSource)->m_map.getNext(true); pUnivType != NULL; pUnivType = const_cast<BaseDB *>(pSource)->m_map.getNext()){
			UnivType& newobj = addNew(pUnivType->getName());
			newobj.copy(*pUnivType);
			ReplaceList.push_back(std::pair<UnivType *, UnivType *>(const_cast<NamedUnivType *>(pUnivType), &newobj));
		}
		// �T�u�Z�b�g���ł̎Q�Ƃ��O��Ă���̂����ɖ߂�
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

	// �y���zm_pRoot�������ɃR�s�[���Ă��܂��ƌ����J�n�n�_���ς���Ă��܂��̂Ń_��
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

// �v�f�̑��f�[�^�x�[�X����̈ړ�
CDB_ERR BaseDB::move(const char *pName, BaseDB*  pSource)
{
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

CDB_ERR BaseDB::remove(const char *pName)
{
	// TODO : �폜�̑O�ɁA����obj���Q�Ƃ��Ă�����̂�S��invalidate���Ȃ��Ă͂����Ȃ�
	// �������́A�Q�Ƃ���Ă�����폜�ł��Ȃ��悤�ɂ���K�v������

	// ���g��DB���猟��
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
	// ���ɓ����̃I�u�W�F�N�g�����݂��Ă�����G���[�Ƃ���
	if(!confirmAddEnable(pName)){
		return CDB_KEY_COLLISION;
	}

	// �I�u�W�F�N�g�ɖ��O��ݒ�
	NamedUnivType *pNamedTarget = dynamic_cast<NamedUnivType *>(pTarget);
	//NamedUnivType *pNamedTarget = reinterpret_cast<NamedUnivType *>(pTarget);
	pNamedTarget->setName(pName);

	if(m_nonameList.erase(pTarget)){
		// ���Ƃ��Ɩ��O�Ȃ��I�u�W�F�N�g�ł������ꍇ
		m_map.add(pNamedTarget->getName(), pNamedTarget);
	}else{
		// ���Ƃ��Ɩ��O���I�u�W�F�N�g�ł������ꍇ
		pNamedTarget->setName(pName);
	}

	return CDB_OK;
}

void BaseDB::issueNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	// ���[�g�ȉ��̑S�Ă̗v�f�ɍX�V�ʒm����
	receiveNotifyUpdated(pUnivType->getReferenceRoot(true), kind);
}

bool BaseDB::receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	// ���[�g�ȉ��̑S�Ă̗v�f�ɍX�V�ʒm����
	return m_pRoot->receiveNotifyUpdatedChildren(pUnivType, kind);
}

void BaseDB::update()
{
	// ���[�g�ȉ��̑S�Ă̗v�f���X�V����
	m_pRoot->updateChildren();
}

bool BaseDB::receiveNotifyUpdatedChildren(const UnivType *pUnivType, enum UpdateKind kind)
{
	int i, n;
	bool updated = false;
	// ���g�̍X�V���X�g���X�V
	n = m_updateList.size();
	for(i = 0; i < n; i++){
		if(m_updateList[i]->receiveNotifyUpdated(pUnivType, kind)){
			updated = true;
		}
	}
	// ���g�̌������X�g���X�V
	n = m_searchList.size();
	for(i = 0; i < n; i++){
		if(m_searchList[i]->receiveNotifyUpdatedChildren(pUnivType, kind)){
			updated = true;
		}
	}
	return updated;
}

// �Q�Ɛ�̒u������
bool BaseDB::replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith)
{
	bool updated = false;
	// 120728 �X�V���X�g���̃I�u�W�F�N�g�͒u���������s��Ȃ�
	// replaceReference�̗p�r�ł���A�T�u�Z�b�g�̃R�s�[�ɂ����ẮA�K�v���Ȃ�����
	// ���������A�X�V���X�g�����̂��߂ɑ��݂��Ă���̂��s��(�K�v�Ȃ��Ȃ�Ώ����Ă�������)

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
	// ���g�̍X�V���X�g���X�V
	n = m_updateList.size();
	for(i = 0; i < n; i++){
		m_updateList[i]->update();
		n = m_updateList.size();
	}
	// ���g�̌������X�g���X�V
	n = m_searchList.size();
	for(i = 0; i < n; i++){
		m_searchList[i]->updateChildren();
	}
}

// �S�v�f�̏���
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

// �L�[�̑��݊m�F
// internal = false�̂Ƃ��ɂ́A���������N��̌������s��
bool BaseDB::isExistKey(const char *pName, bool internal/* = false*/) const 
{
	const UnivType *pUnivType;
	if(internal){
		// ���g��DB���猟��
		pUnivType = m_map.find(pName);
	}else{
		// �ڑ�����Ă���S�̂�DB���猟��
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
