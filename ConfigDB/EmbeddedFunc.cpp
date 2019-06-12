#include "EmbeddedFunc.h"
#include "CdbException.h"
#include "CommonUtils.h"
#include "UnivTypeOps.h"
#include "Equation.h"
#include <cmath>

// 組み込み関数ID
enum EmbeddedFuncId{
	EFI_INTERPRET = 0,	// 【注意】EFI_INTERPRETは0番以外にしないこと。0番だとParserによって自動的に第２引数にDBが設定される。
	EFI_LET,
	EFI_BITFIELD,
	EFI_SUM,
	EFI_CEIL,
	EFI_FIX,
	EFI_LOG,
	EFI_LOG10,
	EFI_LOG2,
	EFI_SIN,
	EFI_COS,
	EFI_TAN,
	EFI_ASIN,
	EFI_ACOS,
	EFI_ATAN,
	EFI_SINH,
	EFI_COSH,
	EFI_TANH,
	EFI_INC_IF_ODD,
	EFI_FLIPLR,
	EFI_FLIPUD,
	EFI_REPMAT,
	EFI_FIND,
	EFI_ONES,
	EFI_ZEROS,
	EFI_COND,
	EFI_SIZEOF,
	EFI_GET_SUBSET,
	EFI_TO_INT,
	EFI_TO_UINT,
	EFI_TO_REAL,
	EFI_TO_DOUBLE,
	EFI_TO_BOOL,
	EFI_TO_STRING,
	EFI_SPRINTF,

	EFI_INVALID
};

// 依存引数指定フラグ
enum DependArg{
	DA_NONE = 0x00000, 
	DA_ARG1 = 0x00001, 
	DA_ARG2 = 0x00002, 
	DA_ARG3 = 0x00004, 
	DA_ARG4 = 0x00008, 
	DA_ARG5 = 0x00010, 
	DA_ARG6 = 0x00020, 
	DA_ARG7 = 0x00040, 
	DA_ARG8 = 0x00080, 
	DA_ARG9 = 0x00100, 
	DA_ARG10 = 0x00200, 
	DA_ARG11 = 0x00400, 
	DA_ARG12 = 0x00800, 
	DA_ARG13 = 0x01000, 
	DA_ARG14 = 0x02000, 
	DA_ARG15 = 0x04000, 
	DA_ARG16 = 0x08000, 
	DA_ALL = 0x0ffff
};

static void Let(UnivType& result, const UnivType **pArgs, int nArg);
static void BitField(UnivType& result, const UnivType **pArgs, int nArg);
static void Sum(UnivType& result, const UnivType **pArgs, int nArg);
static void Ceil(UnivType& result, const UnivType **pArgs, int nArg);
static void Fix(UnivType& result, const UnivType **pArgs, int nArg);
static void Log(UnivType& result, const UnivType **pArgs, int nArg);
static void Log2(UnivType& result, const UnivType **pArgs, int nArg);
static void Log10(UnivType& result, const UnivType **pArgs, int nArg);
static void Sin(UnivType& result, const UnivType **pArgs, int nArg);
static void Cos(UnivType& result, const UnivType **pArgs, int nArg);
static void Tan(UnivType& result, const UnivType **pArgs, int nArg);
static void ArcSin(UnivType& result, const UnivType **pArgs, int nArg);
static void ArcCos(UnivType& result, const UnivType **pArgs, int nArg);
static void ArcTan(UnivType& result, const UnivType **pArgs, int nArg);
static void HypSin(UnivType& result, const UnivType **pArgs, int nArg);
static void HypCos(UnivType& result, const UnivType **pArgs, int nArg);
static void HypTan(UnivType& result, const UnivType **pArgs, int nArg);
static void IncIfOdd(UnivType& result, const UnivType **pArgs, int nArg);
static void FlipLR(UnivType& result, const UnivType **pArgs, int nArg);
static void FlipUD(UnivType& result, const UnivType **pArgs, int nArg);
static void Repmat(UnivType& result, const UnivType **pArgs, int nArg);
static void Find(UnivType& result, const UnivType **pArgs, int nArg);
static void Ones(UnivType& result, const UnivType **pArgs, int nArg);
static void Zeros(UnivType& result, const UnivType **pArgs, int nArg);
static void Cond(UnivType& result, const UnivType **pArgs, int nArg);
static void SizeOf(UnivType& result, const UnivType **pArgs, int nArg);
static void GetSubset(UnivType& result, const UnivType **pArgs, int nArg);
static void Interpret(UnivType& result, const UnivType **pArgs, int nArg);
static void ToInt(UnivType& result, const UnivType **pArgs, int nArg);
static void ToUInt(UnivType& result, const UnivType **pArgs, int nArg);
static void ToReal(UnivType& result, const UnivType **pArgs, int nArg);
static void ToDouble(UnivType& result, const UnivType **pArgs, int nArg);
static void ToBool(UnivType& result, const UnivType **pArgs, int nArg);
static void ToString(UnivType& result, const UnivType **pArgs, int nArg);
static void Sprintf(UnivType& result, const UnivType **pArgs, int nArg);

struct EmbeddedFuncMap g_EmbeddedFuncMap[] = {
	{EFI_INTERPRET,  "eval",         Interpret,    DA_ARG1, DA_NONE},	// 文字列の構文解析＆評価
	{EFI_LET,        "let",          Let,          DA_ALL,  DA_NONE},	// 代入
	{EFI_BITFIELD,   "BitField",     BitField,     DA_ALL,  DA_NONE},	// ビットフィールド
	{EFI_SUM,        "sum",          Sum,          DA_ALL,  DA_NONE},	// 合計
	{EFI_CEIL,       "ceil",         Ceil,         DA_ALL,  DA_NONE},	// 切り上げ
	{EFI_FIX,        "fix",          Fix,          DA_ALL,  DA_NONE},	// 切捨て
	{EFI_LOG,        "log",          Log,          DA_ALL,  DA_NONE},	// Log(底は自然対数の底)
	{EFI_LOG10,      "log10",        Log10,        DA_ALL,  DA_NONE},	// Log(底は10)
	{EFI_LOG2,       "log2",         Log2,         DA_ALL,  DA_NONE},	// Log(底は2)
	{EFI_SIN,        "sin",          Sin,          DA_ALL,  DA_NONE},	// Sin
	{EFI_COS,        "cos",          Cos,          DA_ALL,  DA_NONE},	// Cos
	{EFI_TAN,        "tan",          Tan,          DA_ALL,  DA_NONE},	// Tan
	{EFI_ASIN,       "arcsin",       ArcSin,       DA_ALL,  DA_NONE},	// ArcSin
	{EFI_ACOS,       "arccos",       ArcCos,       DA_ALL,  DA_NONE},	// ArcCos
	{EFI_ATAN,       "arctan",       ArcTan,       DA_ALL,  DA_NONE},	// ArcTan
	{EFI_SINH,       "sinh",         HypSin,       DA_ALL,  DA_NONE},	// HyperbolicSin
	{EFI_COSH,       "cosh",         HypCos,       DA_ALL,  DA_NONE},	// HyperbolicCos
	{EFI_TANH,       "tanh",         HypTan,       DA_ALL,  DA_NONE},	// HyperbolicTan
	{EFI_INC_IF_ODD, "IncIfOdd",     IncIfOdd,     DA_ALL,  DA_NONE},	// 奇数ならインクリメント
	{EFI_FLIPLR,     "fliplr",       FlipLR,       DA_NONE, DA_NONE},	// 左右反転
	{EFI_FLIPUD,     "flipud",       FlipUD,       DA_NONE, DA_NONE},	// 上下反転
	{EFI_REPMAT,     "repmat",       Repmat,       DA_ARG2 | DA_ARG3, DA_ARG1},	// 繰り返しコピー
	{EFI_FIND,       "find",         Find,         DA_ALL,  DA_ARG2},	// 要素を見つける
	{EFI_ONES,       "ones",         Ones,         DA_ALL,  DA_NONE},	// 全て１の配列を生成
	{EFI_ZEROS,      "zeros",        Zeros,        DA_ALL,  DA_NONE},	// 全て０の配列を生成
	{EFI_COND,       "cond",         Cond,         DA_ARG1, DA_NONE},	// 条件による選択
	{EFI_SIZEOF,     "sizeof",       SizeOf,       DA_ARG2, DA_ARG1},	// 配列のサイズを取得
	{EFI_GET_SUBSET, "GetSubset",    GetSubset,    DA_ALL,  DA_NONE},	// サブセット変数の取得
	{EFI_TO_INT,     "ToInt",        ToInt,        DA_ALL,  DA_NONE},	// Intキャスト
	{EFI_TO_UINT,    "ToUInt",       ToUInt,       DA_ALL,  DA_NONE},	// UIntキャスト
	{EFI_TO_REAL,    "ToReal",       ToReal,       DA_ALL,  DA_NONE},	// Realキャスト
	{EFI_TO_DOUBLE,  "ToDouble",     ToDouble,     DA_ALL,  DA_NONE},	// Doubleキャスト
	{EFI_TO_BOOL,    "ToBool",       ToBool,       DA_ALL,  DA_NONE},	// Boolキャスト
	{EFI_TO_STRING,  "ToString",     ToString,     DA_ALL,  DA_NONE},	// Stringキャスト
	{EFI_SPRINTF,    "sprintf",      Sprintf,      DA_ALL,  DA_NONE},	// sprintf
	{EFI_INVALID,    NULL,           NULL,         DA_NONE, DA_NONE}
};

// internal function prototypes ------------------------------

static void ScalarCeil(UnivType& result, const UnivType& x);
static void ScalarFix(UnivType& result, const UnivType& x);
static void ScalarIncIfOdd(UnivType& result, const UnivType& x);
static inline void ScalarRealDblFunc(UnivType& result, const UnivType& x, UnivType::Real (*pfnReal)(UnivType::Real), UnivType::Double (*pfnDbl)(UnivType::Double));
static void ScalarLog(UnivType& result, const UnivType& x);
static void ScalarLog10(UnivType& result, const UnivType& x);
static void ScalarLog2(UnivType& result, const UnivType& x);
static void ScalarSin(UnivType& result, const UnivType& x);
static void ScalarCos(UnivType& result, const UnivType& x);
static void ScalarTan(UnivType& result, const UnivType& x);
static void ScalarArcSin(UnivType& result, const UnivType& x);
static void ScalarArcCos(UnivType& result, const UnivType& x);
static void ScalarArcTan(UnivType& result, const UnivType& x);
static void ScalarHypSin(UnivType& result, const UnivType& x);
static void ScalarHypCos(UnivType& result, const UnivType& x);
static void ScalarHypTan(UnivType& result, const UnivType& x);
static void ScalarRealOp(UnivType& result, const UnivType& x);
template <class T>
static void ScalarCast(UnivType& result, const UnivType& x);
static void ScalarInterpret(UnivType& result, const UnivType& x);

// Scalar Function Implementations ------------------------------

static void ScalarCeil(UnivType& result, const UnivType& x)
{
	result = (int)ceil(static_cast<UnivType::Real>(x));
}

static void ScalarFix(UnivType& result, const UnivType& x)
{
	result = static_cast<int>(static_cast<UnivType::Real>(x));
}

static void ScalarIncIfOdd(UnivType& result, const UnivType& x)
{
	result = ((int(x) & 1) == 1) ? (int(x) + 1) : int(x);
}

static inline void ScalarRealDblFunc(UnivType& result, const UnivType& x, UnivType::Real (*pfnReal)(UnivType::Real), UnivType::Double (*pfnDbl)(UnivType::Double))
{
	if(x.isDouble()){
		result = (UnivType::Double)(*pfnDbl)(x.getDouble());
	}else{
		result = (UnivType::Real)(*pfnReal)(x.getReal());
	}
}

static void ScalarLog(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, log, log);
}

static void ScalarLog10(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, log10, log10);
}

static void ScalarLog2(UnivType& result, const UnivType& x)
{
	if(x.isDouble()){
		result.setDouble(static_cast<UnivType::Double>(log(x.getDouble()) / log(2.0)));
	}else{
		result.setReal(static_cast<UnivType::Real>(log(x.getReal()) / log(2.0)));
	}
}

static void ScalarSin(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, sin, sin);
}

static void ScalarCos(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, cos, cos);
}

static void ScalarTan(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, tan, tan);
}

static void ScalarArcSin(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, asin, asin);
}

static void ScalarArcCos(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, acos, acos);
}

static void ScalarArcTan(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, atan, atan);
}

static void ScalarHypSin(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, sinh, sinh);
}

static void ScalarHypCos(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, cosh, cosh);
}

static void ScalarHypTan(UnivType& result, const UnivType& x)
{
	ScalarRealDblFunc(result, x, tanh, tanh);
}

template <class T>
static void ScalarCast<T>(UnivType& result, const UnivType& x)
{
	result = (T)x;
}

static void ScalarCastToString(UnivType& result, const UnivType& x)
{
	x.toString(result);
}

static void ScalarCastToBinary(UnivType& result, const UnivType& x)
{
	x.toBinary(result);
}

static void ScalarCastToSubsetDB(UnivType& result, const UnivType& x)
{
	x.toSubsetDB(result);
}

// Implementation of interpreter ------------------------------

#include "TokenAnaly.h"
#include "Parser.h"
#include "RequireUpdateObj.h"

// x : description string of equation
// y : subset
static void ScalarInterpret(UnivType& result, const UnivType& x, const UnivType& y)
{
	UnivType text;
	text.evalCopy(x);

	// create token object
	TokenAnaly token(text.getString());

	BaseDB *pDB = y.getSubsetDB();
	RequireUpdateObj::setDB(pDB);

	// create parser object
	Parser parser(token, *pDB);

	// parse as right-side equation
	parser.interpret(result);
}

// Interface Function Implementations ------------------------------

static void Let(UnivType& result, const UnivType **pArgs, int nArg)
{
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

static void BitField(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 2) throw CdbException(CDB_CFUNC_INVALID_NARG, "BitField");
	int n = pArgs[0]->getNumElements();
	if(n != pArgs[1]->getNumElements()) throw CdbException(CDB_CFUNC_INVALID_ARG2, "BitField", "Array size mismatch");
	unsigned int s = 0U;
	unsigned int x = 0U;
	for(int i = 0; i < n; i++){
		unsigned int fsize = (unsigned int)pArgs[0]->getAt(i);
		unsigned int mask = (1U << fsize) - 1;
		x += ((unsigned int)pArgs[1]->getAt(i) & mask) << s;
		s += fsize;
	}
	result = x;
}

static void Sum(UnivType& result, const UnivType **pArgs, int nArg)
{
	UnivType temp;
	int i, n;
	switch(nArg){
	case 1:
		n = pArgs[0]->getNumElements();
		result.evalCopy(pArgs[0]->getAt(0));
		for(i = 1; i < n; i++){
			temp.move(result);
			AopPlus(result, temp, pArgs[0]->getAt(i));
		}
		break;
	case 2:
		{
			int row = pArgs[0]->getNumRow();
			int col = pArgs[0]->getNumColumn();
			int mode = (int)(*pArgs[1]);
			if(mode == 1){
				// 行方向の和
				result.setMatrix(row, 1);
				for(i = 0; i < row; i++){
					UnivType& sum = result[i];
					sum.evalCopy(pArgs[0]->getAt(i, 0));
					for(int j = 1; j < col; j++){
						temp.move(result);
						AopPlus(result, temp, pArgs[0]->getAt(i, j));
					}
				}
			}else if(mode == 2){
				// 行方向の和
				result.setMatrix(1, col);
				for(i = 0; i < col; i++){
					UnivType& sum = result[i];
					sum.evalCopy(pArgs[0]->getAt(0, i));
					for(int j = 1; j < row; j++){
						temp.move(result);
						AopPlus(result, temp, pArgs[0]->getAt(j, i));
					}
				}
			}else{
				throw CdbException(CDB_CFUNC_INVALID_ARG, "Sum");
			}			
		}
		break;
	case 3:
		{
			int start = (int)(*pArgs[1]);
			n = (int)(*pArgs[2]);
			if(n > 0){
				result.evalCopy(pArgs[0]->getAt(start));
				for(i = 1; i < n; i++){
					temp.move(result);
					AopPlus(result, temp, pArgs[0]->getAt(start + i));
				}
			}else{
				result = 0;
			}
		}
		break;
	default:
		throw CdbException(CDB_CFUNC_INVALID_NARG, "Sum");
	}
}

static void Ceil(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Ceil");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCeil);
}

static void Fix(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Fix");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarFix);
}

static void Log(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Log");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarLog);
}

static void Log2(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Log2");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarLog2);
}

static void Log10(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Log10");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarLog10);
}

static void Sin(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Sin");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarSin);
}

static void Cos(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Cos");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCos);
}

static void Tan(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "Tan");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarTan);
}

static void ArcSin(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ArcSin");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarArcSin);
}

static void ArcCos(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ArcCos");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarArcCos);
}

static void ArcTan(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ArcTan");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarArcTan);
}

static void HypSin(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "HypSin");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarHypSin);
}

static void HypCos(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "HypCos");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarHypCos);
}

static void HypTan(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "HypTan");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarHypTan);
}

static void IncIfOdd(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "IncIfOdd");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarIncIfOdd);
}

static void FlipLR(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "FlipLR");
#if 0	// 後で実装
	IndexConv *pConv = new IndexConv;
	pConv->setConvTypeMatrix(DIM_COL, ICT_REVERSE);
	result.setReference(pArgs, pConv);
#endif
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

static void FlipUD(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "FlipUD");
#if 0	// 後で実装
	IndexConv *pConv = new IndexConv;
	pConv->setConvTypeMatrix(DIM_ROW, ICT_REVERSE);
	result.setReference(pArgs, pConv);
#endif
	throw CdbException(CDB_NOT_IMPLEMENTED);
}

static void Repmat(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 3) throw CdbException(CDB_CFUNC_INVALID_NARG, "Repmat");
	int row = pArgs[0]->getNumRow();
	int col = pArgs[0]->getNumColumn();
	int nRowRatio = (int)(*pArgs[1]);
	int nColRatio = (int)(*pArgs[2]);
	if((nRowRatio < 0) || (nColRatio < 0)) throw CdbException(CDB_CFUNC_INVALID_ARG, "Repmat");
	result.setMatrix(row * nRowRatio, col * nColRatio);
	int i, j;
	int p, q;
	int m, n;
	for(p = 0, m = 0; p < nRowRatio; p++){
		for(i = 0; i < row; i++, m++){
			for(q = 0, n = 0; q < nColRatio; q++){
				for(j = 0; j < col; j++, n++){
					result.getAt(m, n).setConstReference(&pArgs[0]->getAt(i, j));
				}
			}
		}
	}
}

static void Find(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 2) throw CdbException(CDB_CFUNC_INVALID_NARG, "Find");
	const UnivType& e = *pArgs[0];
	const UnivType& vec = *pArgs[1];

	int nRow = vec.getNumRow();
	int nCol = vec.getNumColumn();
	int i, j;
	for(j = 0; j < nCol; j++){
		for(i = 0; i < nRow; i++){
			if(vec.getAt(i, j) == e) break;
		}
	}

	result.setMatrix(2, 1);
	result[0] = (i == nRow) ? -1 : i;
	result[1] = (j == nCol) ? -1 : j;
}

static void Ones(UnivType& result, const UnivType **pArgs, int nArg)
{
	int i, n;
	switch(nArg){
	case 2:
		result.setMatrix((int)(*pArgs[0]), (int)(*pArgs[1]));
		n = result.getNumElements();
		for(i = 0; i < n; i++) result[i] = 1;
		break;
	case 1:
		result.setMatrix((int)(*pArgs[0]), 1);
		n = result.getNumElements();
		for(i = 0; i < n; i++) result[i] = 1;
		break;
	default:
		throw CdbException(CDB_CFUNC_INVALID_NARG, "Ones");
	}
}

static void Zeros(UnivType& result, const UnivType **pArgs, int nArg)
{
	int i, n;
	switch(nArg){
	case 2:
		result.setMatrix((int)(*pArgs[0]), (int)(*pArgs[1]));
		n = result.getNumElements();
		for(i = 0; i < n; i++) result[i] = 0;
		break;
	case 1:
		result.setMatrix((int)(*pArgs[0]), 1);
		n = result.getNumElements();
		for(i = 0; i < n; i++) result[i] = 0;
		break;
	default:
		throw CdbException(CDB_CFUNC_INVALID_NARG, "Zeros");
	}
}

static void Cond(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg == 0) throw CdbException(CDB_CFUNC_INVALID_NARG, "Cond");
	int cond = (int)(*pArgs[0]);
	if(nArg < cond + 2) throw CdbException(CDB_CFUNC_INVALID_NARG, "Cond");
	result.setConstReference(pArgs[cond + 1]);
}

static void SizeOf(UnivType& result, const UnivType **pArgs, int nArg)
{
	if((nArg != 1) && (nArg != 2)) throw CdbException(CDB_CFUNC_INVALID_NARG, "SizeOf");
	Equation *pEquation = new Equation;
	result.setEquation(pEquation);

	pEquation->setOperation(Equation::OP_ARRAY_SIZE);
	pEquation->setNumArguments(nArg);
	pEquation->getArgumentAt(0).setConstReference(pArgs[0]);
	if(nArg == 2){
		pEquation->getArgumentAt(1).setConstReference(pArgs[2]);
	}
}

static void GetSubset(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 2) throw CdbException(CDB_CFUNC_INVALID_NARG, "GetSubset");
	Equation *pEquation = new Equation;
	result.setEquation(pEquation);

	pEquation->setOperation(Equation::OP_SUBSET_ACCESS);
	pEquation->setNumArguments(nArg);
	pEquation->getArgumentAt(0).setConstReference(pArgs[0]);
	pEquation->getArgumentAt(1).setConstReference(pArgs[2]);
}

static void Interpret(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 2) throw CdbException(CDB_CFUNC_INVALID_NARG, "eval");
	ElementOperationScalarAndMatrix(result, *pArgs[0], *pArgs[1], ScalarInterpret);
}

static void ToInt(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ToInt");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCast<int>);
}

static void ToUInt(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ToUInt");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCast<unsigned int>);
}

static void ToReal(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ToReal");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCast<UnivType::Real>);
}

static void ToDouble(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ToDouble");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCast<UnivType::Double>);
}

static void ToBool(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ToBool");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCast<bool>);
}

static void ToString(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg != 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "ToString");
	ElementOperationUnaryMatrix(result, *pArgs[0], ScalarCastToString);
}

// Implementation of sprintf ------------------------------

static void SprintfSingleArg(char *pOutput, const char *pFormat, const UnivType& arg, char arg_type_id)
{
	switch(arg_type_id){
	case'd':
	case'i':
		sprintf(pOutput, pFormat, arg.getInt());
		break;
	case'u':
	case'o':
	case'x':
	case'X':
		sprintf(pOutput, pFormat, arg.getUInt());
		break;
	case'e':
	case'E':
	case'f':
	case'F':
	case'g':
	case'G':
	case'a':
	case'A':
		sprintf(pOutput, pFormat, arg.getDouble());
		break;
	case'%':
	case'"':
		sprintf(pOutput, pFormat);
		break;
	default:
		throw CdbException(CDB_SPRINTF_NOTSUPPORT, pFormat);
	}
}

#define MAXLEN_FOR_SINGLE_ARG 48
static void Sprintf(UnivType& result, const UnivType **pArgs, int nArg)
{
	if(nArg < 1) throw CdbException(CDB_CFUNC_INVALID_NARG, "sprintf");

	const char *pFormat = pArgs[0]->getString();

	int max_len = strlen(pFormat) + MAXLEN_FOR_SINGLE_ARG * (nArg - 1);
	UnivType temp;
	temp.setBinary(NULL, max_len);
	char *pResult = static_cast<char *>(temp.getBinary());
	memset(pResult, '\0', sizeof(char) * max_len);

	char szSingleArgFormat[MAXLEN_FOR_SINGLE_ARG];
	char szSingleArgOutput[MAXLEN_FOR_SINGLE_ARG];

	for(int n = 1; n < nArg; n++){
		while((*pFormat != '\0') && (*pFormat != '%')){
			*pResult++ = *pFormat++;
		}
		if(*pFormat == '\0') break;
		int i = 0;
		szSingleArgFormat[i++] = *pFormat++;
		for(; (*pFormat != '\0') && ((isdigit(*pFormat) != 0) || (*pFormat == '.')); i++, pFormat++){
			szSingleArgFormat[i] = *pFormat;
		}
		if(*pFormat == '\0') break;
		char arg_type_id = *pFormat;
		szSingleArgFormat[i++] = *pFormat++;
		szSingleArgFormat[i++] = '\0';
		SprintfSingleArg(szSingleArgOutput, szSingleArgFormat, *pArgs[n], arg_type_id);
		strcpy(pResult, szSingleArgOutput);
		pResult += strlen(szSingleArgOutput);
	}

	strcpy(pResult, pFormat);
	result.setString(static_cast<char *>(temp.getBinary()));
}
