#ifndef DEF_PARSER
#define DEF_PARSER

class UnivType;
class TokenAnaly;
class BaseDB;
class IndexConv;

enum TokenKind;
class SubsetContext;
class BraceContext;
class SentenceContext;
class IndexContext;
class CatContext;

class Parser
{
private:
	TokenAnaly& m_token;				// トークン取得オブジェクト
	BaseDB& m_RootDB;					// 解析内容の登録先DB
	BaseDB *m_pCustomFuncArguments;		// ユーザー定義関数の引数名登録DB（使わない場合はNULL）

	// トークン操作関数
	enum TokenKind getNextToken();				// 次のトークンの種別を得る
	bool checkNextToken(enum TokenKind kind);	// 次のトークンの種別を判定する（不一致なら１つ戻す）
	bool assertNextToken(enum TokenKind kind);	// 次のトークンの種別を判定する（不一致ならエラー）
	void getCurrentTokenContent(UnivType& obj);	// 現在のトークンの内容を取得する
	void putBackToken();						// 取得したトークンを戻す

	// オブジェクト取得関数
	UnivType& getObjectByName(const char *pName);

private:
	// 現在のコンテキスト保持変数
	SubsetContext *m_pSubsetContext;
	BraceContext *m_pBraceContext;
	SentenceContext *m_pSentenceContext;
	IndexContext *m_pIndexContext;
	CatContext *m_pCatContext;

private:
	// 構文解析関数
	void topScript();					// スクリプト構文解析トップ
	void subsetDefine();				// サブセット(ブロック)定義(グローバル位置、名前無し版)
	void topEquation();					// 式解析トップ
	UnivType& topLeft();				// 左辺式構文解析トップ
	UnivType& topFunc();				// 関数定義式（左辺）構文解析トップ
	void topRight(UnivType& obj);		// 右辺式トップ

	// 右辺式解析関数
	void branch(UnivType& obj);			// 分岐
	void condition(UnivType& obj);		// 条件式
	void matrix(UnivType& obj);			// 行列
	void vector(UnivType& obj);			// ベクトル
	void expression(UnivType& obj);		// 式（＋,－などによる結合）
	void term(UnivType& obj);			// 項（＊,÷などによる結合）
	void unaryOp(UnivType& obj);		// 単項演算（転置、転置共役など）
	void subsetAccess(UnivType& obj);	// サブセットアクセス演算
	bool subsetDeclare(UnivType& obj);	// サブセット定義(右辺式、名前あり版)
	bool func(UnivType& obj);			// 関数・配列アクセス演算
	bool primitive(UnivType& obj);		// 基本要素

	void expression2nd(UnivType& obj, UnivType& first);	// 式（２要素目以降）
	void term2nd(UnivType& obj, UnivType& first);		// 項（２要素目以降）

	// 右辺式解析関数
	void matrixLeft(UnivType& obj);		// 行列
	void vectorLeft(UnivType& obj);		// ベクトル
	void primitiveLeft(UnivType& obj);	// 基本要素

	// 配列インデックス解析関数
	void arrayIndexRight(IndexConv *pIndexConv, const UnivType *pArray, int nDim);	// 配列インデックス（右辺式）
	void arrayIndexLeft(IndexConv *pIndexConv, const UnivType *pArray, int nDim);	// 配列インデックス（左辺式）

public:
	Parser(TokenAnaly& token, BaseDB& database);
	~Parser();

	void execute();	// 解析の実行
	void interpret(UnivType& obj);	// 右辺式の解析の実行
	bool generateReference(UnivType& ref);	// 何のためにあるのか不明

};

#endif
