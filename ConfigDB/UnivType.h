#ifndef DEF_UNIV_TYPE
#define DEF_UNIV_TYPE

#include "CdbException.h"
#include "CommonUtils.h"

//#define UNION_AS_STRUCT	// for debug

class UnivMatrix;
class IndexConv;
class Equation;
class BaseDB;
enum UpdateKind;

class UnivType
{
public:
	// 実数型定義
	typedef float Real;
	typedef double Double;

protected:
	// タイプ識別子
	enum ContentKind{
		TYPE_EMPTY = 0, 
		TYPE_REF, 
		TYPE_REF_CONST, 
		TYPE_MATRIX, 
		TYPE_EQUATION, 
		TYPE_INT, 
		TYPE_UINT, 
		TYPE_REAL, 
		TYPE_DOUBLE, 
		TYPE_POINTER, 
		TYPE_BOOL, 
		TYPE_STRING, 
		TYPE_BINARY, 
		TYPE_SUBSET, 
		TYPE_INVALID
	};

	// 情報保持共用体
#ifdef UNION_AS_STRUCT
	struct CommonType
#else
	union CommonType
#endif
	{
		int integer;
		unsigned int uinteger;
		void *ptr;
		bool boolean;
		Real real;
		Double dreal;
		Equation *pEquation;
		UnivMatrix *pMatrix;
		struct tagRefer{
			UnivType *pRefer;
			IndexConv *pIndexConv;
		}refer;
		struct tagString{
			char *pString;
			size_t length;
		}string;
		struct tagBinData{
			unsigned char *pData;
			size_t size;
		}binData;
		struct tagSubset{
			BaseDB *pDB;
		}subset;
	};

	// 型情報・属性保持
#if 1
	struct TypeInfo{
		unsigned int hasName       :	1;
		unsigned int hasOwner      :    1;
		unsigned int isConstant    :	1;
		unsigned int isObjectOwner :    1;
		unsigned int contentKind   :	4;
	};
#else
	struct TypeInfo{
		unsigned char hasName;			// 名前付きかどうか(NamedUnivTypeにキャスト可能)
		unsigned char hasOwner;			// 所有されているか(OwnedUnivTypeにキャスト可能)
		unsigned char isConstant;		// 定数属性かどうか
		unsigned char isObjectOwner;	// 子オブジェクトが自身の持ち物であるか(trueならば消滅時に開放する)
		unsigned char contentKind;		// タイプ識別子
	};
#endif

protected:
	// データ保持の共用体
#ifdef UNION_AS_STRUCT
	struct CommonType m_data;
#else
	union CommonType m_data;
#endif

	// データの種別とデータサイズ
	struct TypeInfo m_type;
	static struct TypeInfo m_initial_type;

	// サブセットの設定
	//////////////////////////////////////////////////////////////////////////////
	// 【注意】BaseDBのみがサブセットを作成できる。
	// ユーザーはBaseDB::createSubset()を利用すること。
	//////////////////////////////////////////////////////////////////////////////
	// BaseDBのみがサブセットを作成できるため、フレンド指定しておく
	friend BaseDB;
public:
	// 渡されたBaseDBオブジェクトをラップしてsubsetオブジェクトを生成する
	// delegated == true に設定すると、以降の存続管理はUnivTypeに移る
	void setSubset(BaseDB *pDB, bool delegated = true);

private:
	// 数式の評価（例外に変数名を付加して投げなおす）
	inline UnivType& evalEquation() const;
	// 変数名を付加して例外を投げる（配列要素番号は付加されない）
	void throwException(CDB_ERR eErrorCode) const;
	// 参照オブジェクトの取得（インデックス変換の結果、要素数が１になった行列をスカラーとして返す）
	// インデックス変換が存在する場合で、変換の結果に要素数が１でない場合は例外となる
	inline UnivType& getScalarReference();
	inline const UnivType& getScalarReference() const;
	// 参照オブジェクトの取得（インデックス変換の結果、要素数が１になった行列をスカラーとして返す）
	// インデックス変換の結果に要素数が１でない場合は参照オブジェクトそのものを返す
	inline const UnivType& getReferenceForTypeInspection() const;
	//////////////////////////////////////////////////////////////////////////////
	// 【注意】UnivTypeでは、１×１行列とスカラーを別物として扱う。
	// しかし、インデックス変換の結果１×１となる参照行列の場合、文法上の特別規則によってスカラーとして扱う。
	//////////////////////////////////////////////////////////////////////////////
	// 利用直前でない場合、参照においてインデックス変換を伴うかどうかを取得する
	inline bool isIndexConvAvailable() const;
	// 利用直前に、参照においてインデックス変換を伴うかどうかを取得する(同時にインデックス変換の更新も行う)
	inline bool isIndexConvExists() const;

public:
	UnivType(){ Initialize(); }
	UnivType(int x){ Initialize(); setInt(x); }
	UnivType(unsigned int x){ Initialize(); setUInt(x); }
	UnivType(Real x){ Initialize(); setReal(x); }
	UnivType(Double x){ Initialize(); setDouble(x); }
	UnivType(const char *p){ Initialize(); setString(p); }
	UnivType(bool x){ Initialize(); setBool(x); }
	UnivType(UnivType& src) { Initialize(); move(src); }
	//UnivType(const UnivType& src) { Initialize(); copy(src); }
	virtual ~UnivType(){ clearAll(); }

	void Initialize();
	void clearAll();	// フル参照も含めた強制クリア
	void clear();		// 通常のクリア
	void invalidate();
	void copy(const UnivType& src);		// 内容の全コピー	
	void evalCopy(const UnivType& src);	// 数式を評価してから結果をコピー
	void move(UnivType& src);

	// 更新通知の受け取り
	bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	void update();

	// 参照先の置き換え
	bool replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith);

	void setMatrix(int row, int col);
	// 非constオブジェクトに対する非const行列要素の取得
	UnivType& operator[](int n){ return getAt(n); }
	UnivType& getAt(int n);
	UnivType& getAt(int row, int col);
	// constオブジェクトに対するconst行列要素の取得
	const UnivType& operator[](int n) const { return getAtConst(n); }
	const UnivType& getAt(int n) const { return getAtConst(n); }
	const UnivType& getAt(int row, int col) const { return getAtConst(row, col); }
	// 非constオブジェクトに対する明示的なconst行列要素の取得
	const UnivType& getAtConst(int n) const;
	const UnivType& getAtConst(int row, int col) const;

	int getNumRow() const;
	int getNumColumn() const;
	int getNumElements() const;

	int getInt() const;
	unsigned int getUInt() const;
	void *getPointer() const;
	bool getBool() const;
	Real getReal() const;
	Double getDouble() const;
	const char *getString() const;
	size_t getStringLength() const;
	void *getBinary() const;
	size_t getBinarySize() const;
	BaseDB *getSubsetDB() const;

	void toString(UnivType& obj) const;
	void toBinary(UnivType& obj) const;
	void toSubsetDB(UnivType& obj) const;

	char *getFormatString(char *pBuff, size_t sizeBuff) const;
#ifdef DEBUG_PRINT
	void debugPrintStructure(int nIndentLevel, bool collapsed = true) const;
#endif

	void setInt(int x);
	void setUInt(unsigned int x);
	void setPointer(void *p);
	void setBool(bool x);
	void setReal(Real x);
	void setDouble(Double x);
	void setString(const char *p);
	void setBinary(const void *p, size_t size);

	// IndexConvオブジェクトを設定すると、以降の存続管理はUnivTypeに移る(参照先は管理しない)
	//////////////////////////////////////////////////////////////////////////////
	// 【注意】参照設定は、他の設定に優先される。
	// 参照設定されているUnivTypeにsetInt()などで設定を行っても、参照先に設定されるのみである。
	// 参照設定を解除するには、参照先にNULLを指定して解除するしかない。
	//////////////////////////////////////////////////////////////////////////////
	void setFullReference(UnivType *pRefer, IndexConv *pIndexConv = NULL);
	void setConstReference(const UnivType *pRefer, IndexConv *pIndexConv = NULL);
	void getReference(UnivType*& pRefer, IndexConv*& pIndexConv) const;
	void getReference(const UnivType*& pRefer, IndexConv*& pIndexConv) const;

	// 参照の解除(フル参照を解除する手段はこの関数を呼ぶしかない)
	void releaseReference();

	// 参照がネストされている際に、参照されている最先のオブジェクトを得る
	// isGoBackOnlyThroughRef = { true:スルー参照のみを遡る, false:インデックス変換付きも同様に遡る }
	const UnivType *getReferenceRoot(bool isGoBackOnlyThroughRef) const;

	// Equationオブジェクトを設定すると、以降の存続管理はUnivTypeに移る
	void setEquation(Equation *pEquation);
	Equation *getEquation();
	const Equation *getConstEquation() const;

	operator int() const { return getInt(); }
	operator unsigned int() const { return getUInt(); }
	operator short() const { return static_cast<short>(getInt()); }
	operator unsigned short() const { return static_cast<unsigned short>(getUInt()); }
	operator void *() const { return getPointer(); }
	operator bool() const { return getBool(); }
	operator Real() const { return getReal(); }
	operator Double() const { return getDouble(); }
	operator const char *() const { return getString(); }

	UnivType& operator=(int x){ setInt(x); return *this; }
	UnivType& operator=(unsigned int x){ setUInt(x); return *this; }
	UnivType& operator=(Real x){ setReal(x); return *this; }
	UnivType& operator=(Double x){ setDouble(x); return *this; }
	UnivType& operator=(const char *p){ setString(p); return *this; }
	UnivType& operator=(bool x){ setBool(x); return *this; }
	UnivType& operator=(const UnivType& x){ copy(x); return *this; }

	int getContentTypeID() const;

	// 定数の場合の演算の高速化
	void simplify() const;

	// 定数属性(アクセスする際に、書き込みを許可するかどうか)
	bool isConstant() const;
	void setConstant(bool isConstant);

	// fixed属性(依存変数の値の変化に影響される可能性があればtrueを返す)
	bool isFixed() const;

	// 参照であればtrueを返す
	bool isReference() const { return (m_type.contentKind == TYPE_REF) || (m_type.contentKind == TYPE_REF_CONST); }

	// フル参照であればtrueを返す
	bool isFullReference() const { return m_type.contentKind == TYPE_REF; }

	// const(書込み禁止つき)参照であればtrueを返す
	bool isConstReference() const { return m_type.contentKind == TYPE_REF_CONST; }

	// このオブジェクトが有効であればtrueを返す
	bool isValid() const;

	// 何も保持していなければtrueを返す
	bool isEmpty() const;

	// 式であればtrueを返す
	bool isEquation() const;

	// スカラー値（配列でない）ならばtrueを返す
	bool isScalar() const;

	// 行列（またはベクトル）ならばtrueを返す
	bool isMatrix() const;

	// 整数であればtrueを返す
	bool isInteger() const;
	
	// 符号なし整数であればtrueを返す
	bool isUInteger() const;
	
	// 実数であればtrueを返す
	bool isReal() const;
	bool isDouble() const;

	// ポインタであればtrueを返す
	bool isPointer() const;

	// BOOL型であればtrueを返す
	bool isBoolean() const;

	// 文字列であればtrueを返す
	bool isString() const;

	// バイナリオブジェクトであればtrueを返す
	bool isBinary() const;

	// サブセットであればtrueを返す
	bool isSubset() const;

	// 数値であればtrueを返す
	bool isNumber() const { return isInteger() || isUInteger() || isReal() || isDouble(); }

	// 符号なし数値であればtrueを返す
	bool isUnsignedNumber() const { return isUInteger(); }

};

#endif
