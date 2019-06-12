#ifndef DEF_PARSE_CONTEXT
#define DEF_PARSE_CONTEXT

class UnivType;
class IndexConv;
class BaseDB;
class TokenAnaly;
enum TokenKind;

#include "CdbException.h"

#define INVALID  (-1)

// コンテキスト保存構造体
template<class T>
class ParseContext
{
public:
	typedef T *T_PTR;

	ParseContext(T_PTR& pContextContainer) : m_pContextContainer(pContextContainer){}
	~ParseContext(){
		m_pContextContainer = m_pPrevContext;	// 以前のコンテキストに戻す
	}

protected:
	void init(){
#if 0	// 090723 自動パラメータ引継ぎは廃止
		if(m_pContextContainer != NULL){
			memcpy(this, m_pContextContainer, sizeof(T));	// 以前の内容をコピーする(m_pContextContainerへの参照ポインタは、常に同じ値であるため、このような上書きコピーが可能である)
			//*static_cast<T_PTR>(this) = *m_pContextContainer;
		}else{
			setDefault();	// デフォルト値の設定
		}
#else
		if(m_pContextContainer == NULL){
			setDefault();	// デフォルト値の設定
		}
#endif
		m_pPrevContext = m_pContextContainer;				// 以前のコンテキストを保存
		m_pContextContainer = static_cast<T_PTR>(this);		// 新しいコンテキストに切り替える
	}
	virtual void setDefault() = 0;
	T_PTR GetPrevContext() const { return m_pPrevContext; }

private:
	T_PTR& m_pContextContainer;
	T_PTR m_pPrevContext;	// 以前のコンテキストを保存しておく
};

class SubsetContext : public ParseContext<SubsetContext>
{
public:
	SubsetContext(SubsetContext*& p) : ParseContext<SubsetContext>(p){
		if(p != NULL){
			m_pDB = p->m_pDB;
			m_pSubset = p->m_pSubset;
			m_bSubsetOwnerSearch = p->m_bSubsetOwnerSearch;
		}
		init();
	}

protected:
	virtual void setDefault(){
		m_pDB = NULL;
		m_bSubsetOwnerSearch = true;
	}

private:
	BaseDB *m_pDB;				// このコンテキストにおける操作対象SubsetのDB(操作対象Subsetが未決定（実行時に決定）の場合にNULL)
	UnivType *m_pSubset;		// このコンテキストにおける操作対象Subset
	bool m_bSubsetOwnerSearch;	// 所有者のDBまで検索を行うかどうか

	UnivType& runtimeGetFromSubset(const char *pName);

public:
	// DBへのアクセス
	void setRootDB(BaseDB *pDB);			// ルートDBの設定（最初に必ず行っておく）
	void createSubset(const char *pName);	// 指定された名前でサブセットを作成し、それを操作対象とする
	void createSubset(UnivType *pObj);		// 指定されたUnivTypeオブジェクトにサブセットを作成し、それを操作対象とする
	void setSubset(UnivType *pSubset);		// サブセットを指定されたものに切り替える
	UnivType& getSubset();					// 現在の操作対象subsetの取得
	void disableOwnerSearch(){ m_bSubsetOwnerSearch = false; }	// 以降のサブセット所有者への検索を無効化する
	// 以下、BaseDBのラッパー
	UnivType& addNew(const char *pName, CDB_ERR& error);
	UnivType& addNew(const char *pName);
	UnivType& addNew();
	CDB_ERR rename(UnivType *pTarget, const char *pName);
	UnivType& getUnsafeUnivType(const char *pName);
};

class BraceContext : public ParseContext<BraceContext>
{
public:
	BraceContext(BraceContext*& p) : ParseContext<BraceContext>(p) {
		if(p != NULL){
			m_currentBrace = p->m_currentBrace;
			m_countTerm = p->m_countTerm;
			m_countPrimitive = p->m_countPrimitive;
			m_braceLevel = p->m_braceLevel;
		}
		init();
	}

protected:
	virtual void setDefault();

public:
	enum TokenKind m_currentBrace;		// 現在の式が入っている括弧
	short int m_countTerm;				// 式中の何番目の項の中であるか(最初の要素は0番)
	short int m_countPrimitive;			// 項中の何番目の基本要素であるか(最初の要素は0番)
	short int m_braceLevel;				// 構文解析のレベル(最上位から何番目の括弧の中か)
};

class SentenceContext : public ParseContext<SentenceContext>
{
public:
	SentenceContext(SentenceContext*& p) : ParseContext<SentenceContext>(p) {
		if(p != NULL){
			m_isReference = p->m_isReference;
			m_isConstant = p->m_isConstant;
			m_isInsideRight = p->m_isInsideRight;
		}
		init();
	}

protected:
	virtual void setDefault(){
		m_isReference = false;
		m_isConstant = true;
		m_isInsideRight = true;
	}

public:
	bool m_isReference;					// 参照ならtrue(trueのとき、m_isConstantの意味はなくなる)
	bool m_isConstant;					// 定数ならtrue
	bool m_isInsideRight;				// 右辺式中ならtrue
};

class IndexContext : public ParseContext<IndexContext>
{
public:
	IndexContext(IndexContext*& p) : ParseContext<IndexContext>(p) {
		if(p != NULL){
			m_saveTokenPos = p->m_saveTokenPos;
			m_isInsideArrayIndex = p->m_isInsideArrayIndex;
			m_pTargetArray = p->m_pTargetArray;
			m_pIndexConv = p->m_pIndexConv;
			m_arrayDimensionId = p->m_arrayDimensionId;
		}
		init();
	}

protected:
	virtual void setDefault(){
		m_isInsideArrayIndex = false;
		m_pTargetArray = NULL;
	}

private:
	int m_saveTokenPos;					// トークン位置保存用変数

public:
	bool m_isInsideArrayIndex;			// 配列インデックス中であるならtrue
	const UnivType *m_pTargetArray;		// 配列インデックス中である場合、対象の配列へのポインタ
	const IndexConv *m_pIndexConv;		// 配列インデックス中である場合、インデックス変換オブジェクトへのポインタ
	short int m_arrayDimensionId;		// 配列インデックス識別子

	void saveCurrentTokenPos(TokenAnaly& token);	// 現在のトークン位置を保存
	void loadTokenPos(TokenAnaly& token);			// 保存されていたトークン位置に戻す
};

class CatContext : public ParseContext<CatContext>
{
public:
	CatContext(CatContext*& p) : ParseContext<CatContext>(p){
		init();
		// 自動パラメータ引継ぎは行わない 
		// (毎回のcontext生成にてリセットされる)
		setDefault();
	}

protected:
	virtual void setDefault(){
		m_bNestedMatrix = false;
	}

private:
	bool m_bNestedMatrix;			// このコンテキストにおける要素を、catにおいて1要素としてネストする

public:
	// リセット(デフォルト状態に戻す)
	void reset(){ setDefault(); }
#if 0
	// アクセス(1つ前のコンテキストにアクセスする)
	void setNested(bool x){
		if(GetPrevContext() != NULL){
			GetPrevContext()->m_bNestedMatrix = x;
		}
	}
#endif
	// アクセス(現在のコンテキストにアクセスする)
	void setNested(bool x){ m_bNestedMatrix = x; }
	bool getNested(){ return m_bNestedMatrix; }

};

#endif
