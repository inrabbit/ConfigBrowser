#ifndef DEF_BASE_DB
#define DEF_BASE_DB

#include "CommonUtils.h"
#include "WrappedMap.h"
#include "WrappedVector.h"
#include "UnivType.h"
#include "CdbException.h"

class RequireUpdateObj;

class BaseDB
{
private:
	// 接続先のルートDB
	BaseDB *m_pRoot;
	// 自身がサブセットである場合、所有者のDB
	BaseDB *m_pSubsetOwner;
	// 検索DBリスト
	WrappedVector<BaseDB *> m_searchList;
	// 更新リスト(【注意】サブセットは更新リストを持たず、サブセット所有者の更新リストを利用するものとする)
	WrappedVector<RequireUpdateObj *> m_updateList;

	// DBの接続に関するもの
	void connectNamedObjects(BaseDB *pDB);
	void disconnectNamedObjects(BaseDB *pDB);
	bool isConnected(BaseDB *pDB) const;
	static void createLink(BaseDB *pFrom, BaseDB *pTo);
	static void deleteLink(BaseDB *pFrom, BaseDB *pTo);

	// 自身に対する検索要求の処理
	UnivType *search(const char *pName, bool bSubsetOwnerSearch = true);
	// 自身と自身が持つDBの検索
	UnivType *searchSelf(const char *pName, BaseDB *pSearchExcept = NULL);
	// 自身のDBに指定した名前で要素が追加できるかの確認
	bool confirmAddEnable(const char *pName);

protected:
	// 参照返しがエラーの際に返すダミー
	static UnivType m_dummy;

	WrappedMap m_map;
	WrappedVector<UnivType *> m_nonameList;

public:
	// 名前付き新規オブジェクトの登録
	UnivType& addNew(const char *pName, CDB_ERR& error);
	// 名前付き新規オブジェクトの登録（エラーを例外で返す）
	UnivType& addNew(const char *pName);
	// 名前なし新規オブジェクトの登録（エラーを例外で返す）
	UnivType& addNew();

	// サブセットの新規登録（エラーを例外で返す）
	BaseDB *createSubset(UnivType *pObj);

public:
	BaseDB();
	~BaseDB();

	// DBの接続（検索範囲の追加）
	CDB_ERR connectDB(BaseDB *pDB);
	// DBの接続解除
	CDB_ERR disconnectDB(BaseDB *pDB = NULL);

	// コンテナクラスを取得する（デバッグ用一覧表示のため）
	WrappedMap *getContainer(){ return &m_map; }

	// 全要素の消去
	void clear();
	// キーの存在確認
	bool isExistKey(const char *pName, bool internal = false) const;
	// 要素の他データベースからのコピー
	CDB_ERR copy(const char *pName, const BaseDB*  pSource);
	// 全要素の他データベースからのコピー
	CDB_ERR copy(const BaseDB*  pSource);
	CDB_ERR evalCopy(const BaseDB*  pSource);
	// 要素の他データベースからの移動
	CDB_ERR move(const char *pName, BaseDB*  pSource);
	// 要素の削除
	CDB_ERR remove(const char *pName);
	// 要素の名前の変更
	CDB_ERR rename(const char *pOldName, const char *pNewName);
	// 名前なしオブジェクトに名前をつける（又はその逆）
	CDB_ERR rename(UnivType *pTarget, const char *pName);

	// UnivType型（エラーを例外で返す）
	UnivType& getUnivType(const char *pName);	// 定数属性にはアクセスできない
	const UnivType& getConstUnivType(const char *pName) const;
	// UnivType型（エラーコードで返す）
	UnivType& getUnivType(const char *pName, CDB_ERR& error);	// 定数属性にはアクセスできない
	const UnivType& getConstUnivType(const char *pName, CDB_ERR& error) const;
	// UnivType型（エラーを例外で返す）
	// 定数属性でも取得できる(ただし、内容を書き換えようとした時点で例外)
	UnivType& getUnsafeUnivType(const char *pName, bool bSubsetOwnerSearch = true);

	// 整数値
	CDB_ERR getInt(const char *pName, int& x);
	CDB_ERR setInt(const char *pName, int x);
	// 符号なし整数値
	CDB_ERR getUInt(const char *pName, unsigned int& x);
	CDB_ERR setUInt(const char *pName, unsigned int x);
	// 実数値
	CDB_ERR getReal(const char *pName, UnivType::Real& x);
	CDB_ERR setReal(const char *pName, UnivType::Real x);
	// ブール値
	CDB_ERR getBool(const char *pName, bool& x);
	CDB_ERR setBool(const char *pName, bool x);
	// ポインタ値
	CDB_ERR getPtr(const char *pName, void*& p);
	CDB_ERR setPtr(const char *pName, void* p);
	// 文字列
	CDB_ERR getString(const char *pName, const char*& p);
	CDB_ERR setString(const char *pName, const char* p);
	// バイナリデータ
	CDB_ERR getBinary(const char *pName, void*& p, size_t& size);
	CDB_ERR setBinary(const char *pName, const void* p, size_t size);
	// 配列オブジェクト（過去バージョンとの互換性のため。利用は推奨されない。）
	CDB_ERR getArrayObj(const char *pName, void*& p, size_t& size);
	CDB_ERR setArrayObj(const char *pName, const void* p, size_t size);

	// 更新通知の発行
	void issueNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	// 更新通知の受け取り
	bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	void update();
	// 参照先の置き換え
	bool replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith);

#ifdef DEBUG_PRINT
	void debugPrintStructure(int nIndentLevel) const;
#endif

private:
	// 更新通知の受け取り（自身の所有ツリー以下のみ）
	bool receiveNotifyUpdatedChildren(const UnivType *pUnivType, enum UpdateKind kind);
	void updateChildren();

	friend RequireUpdateObj;
	// オブジェクトを更新リストに追加・削除
	void addToUpdateList(RequireUpdateObj *pObj){ m_updateList.push_back(pObj); }
	void removeFromUpdateList(RequireUpdateObj *pObj){ m_updateList.erase(pObj); }

};

#endif
