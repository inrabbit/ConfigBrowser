#include "Equation.h"
#include "CommonUtils.h"
#include "UnivTypeOps.h"
#include "EmbeddedFunc.h"
#include "IndexConv.h"
#include "NamedUnivType.h"
#include <cassert>
#include <cstring>

// 依存引数の設定 --------------------

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

struct FuncDependFlag{
	Equation::EqnOperation m_id;	// 関数ID
	unsigned int m_DependArgsValue;	// 結果が依存する引数の指定(値のみの変更)
	unsigned int m_DependArgsSize;	// 結果が依存する引数の指定(行列のサイズ変更)
};

struct FuncDependFlag Equation::g_FuncDependList[] = {
	{	OP_CUSTOM_FUNC,         DA_ALL, DA_ALL },
	{	OP_CUSTOM_FUNC_CALL,    DA_ALL, DA_ALL },
	{	OP_PLUS,                DA_ALL, DA_ALL },
	{	OP_MULT,                DA_ALL, DA_ALL },
	{	OP_DOT_MULT,            DA_ALL, DA_ALL },
	{	OP_BIT_AND,             DA_ALL, DA_ALL },
	{	OP_BIT_OR,              DA_ALL, DA_ALL },
	{	OP_LOGICAL_AND,         DA_ALL, DA_ALL },
	{	OP_LOGICAL_OR,          DA_ALL, DA_ALL },
	{	OP_CAT_ROW,             DA_NONE, DA_ALL },
	{	OP_CAT_ROW_CONST,       DA_NONE, DA_ALL },
	{	OP_CAT_COLUMN,          DA_NONE, DA_ALL },
	{	OP_CAT_COLUMN_CONST,    DA_NONE, DA_ALL },
	{	OP_CAT_PARTIAL_REF,     DA_NONE, DA_ALL },
	{	OP_SERIES,              DA_ALL, DA_NONE },
	{	OP_ARRAY_SIZE,          DA_ARG2 | DA_ARG3, DA_ARG1 },
	{	OP_IF,                  DA_ARG1, DA_NONE },
	{	OP_IF_CONST,            DA_ARG1, DA_NONE },
	{	OP_COND,                DA_ARG1, DA_NONE },
	{	OP_COND_CONST,          DA_ARG1, DA_NONE },
	{	OP_SWITCH,              DA_ARG1, DA_NONE },
	{	OP_SWITCH_CONST,        DA_ARG1, DA_NONE },
		// binary operations
	{	OP_MINUS,               DA_ALL, DA_ALL },
	{	OP_DIV,                 DA_ALL, DA_ALL },
	{	OP_DOT_DIV,             DA_ALL, DA_ALL },
	{	OP_POWER,               DA_ALL, DA_ALL },
	{	OP_DOT_POWER,           DA_ALL, DA_ALL },
	{	OP_BIT_XOR,             DA_ALL, DA_ALL },
	{	OP_LOGICAL_XOR,         DA_ALL, DA_ALL },
	{	OP_EQUAL,               DA_ALL, DA_ALL },
	{	OP_NEQUAL,              DA_ALL, DA_ALL },
	{	OP_LARGE,               DA_ALL, DA_ALL },
	{	OP_SMALL,               DA_ALL, DA_ALL },
	{	OP_ELARGE,              DA_ALL, DA_ALL },
	{	OP_ESMALL,              DA_ALL, DA_ALL },
	{	OP_MOD,                 DA_ALL, DA_ALL },
	{	OP_SUBSET_ACCESS,       DA_ALL, DA_ALL },
		// unary operations
	{	OP_UNARY_MINUS,         DA_ALL, DA_ALL },
	{	OP_RECIPROCAL,          DA_ALL, DA_ALL },
	{	OP_BIT_NOT,             DA_ALL, DA_ALL },
	{	OP_LOGICAL_NOT,         DA_ALL, DA_ALL },
	{	OP_CONJ,                DA_ALL, DA_ALL },
	{	OP_THROUGH_TILL_WRITE,  DA_NONE, DA_NONE },
		// zero-ary operations
	{	OP_NONE,                DA_NONE, DA_NONE },
		// invalid operation
	{	OP_INVALID,             DA_NONE, DA_NONE }
};

// ------------------------------

// 組み込み関数IDの開始番号
#define EFM_ID_BEGIN	300

// 一度に確保する引数配列のブロック数(2のべき乗数とすること)
#define ARG_ARRAY_ELEMENTS 2


Equation::Equation()
{
	m_eOperation = OP_INVALID;
	m_FixCacheWhenWritten = false;

	m_pArguments = NULL;
	m_pArgArray = NULL;
	m_nArguments = 0;
}

Equation::~Equation()
{
	clearArguments();
}

// 引数を全てクリアする
void Equation::clearArguments()
{
	if(isVariableArgumentNum()){
		assert(m_pArguments != NULL);
		delete[] m_pArguments;	// ポインタ配列の開放
		m_pArguments = NULL;

		assert(m_pArgArray != NULL);
		int nBlock = (m_nArguments + ARG_ARRAY_ELEMENTS / 2) / ARG_ARRAY_ELEMENTS;
		for(int i = 0; i < nBlock; i++){
			delete[] m_pArgArray[i];	// 配列実体の開放
		}
		delete[] m_pArgArray;	// ポインタ配列の開放
		m_pArgArray = NULL;

		m_nArguments = 0;
	}else if(isFixedArgumentNum()){
		assert(m_pArgArray == NULL);

		delete[] m_pArguments[0];
		delete[] m_pArguments;
		m_pArguments = NULL;

		m_nArguments = 0;
	}
}

void Equation::setNumArgumentsVariable(int n)
{
	if(isFixedArgumentNum() || (m_nArguments != n)){
		clearArguments();
	}

	if((m_nArguments != n) && (n > 0)){
		int i, j, k;

		m_nArguments = n;

		int nBlock = (m_nArguments + ARG_ARRAY_ELEMENTS / 2) / ARG_ARRAY_ELEMENTS;
		m_pArgArray = new UnivType*[nBlock];	// ポインタ配列の確保
		for(i = 0; i < nBlock; i++){
			m_pArgArray[i] = new UnivType[ARG_ARRAY_ELEMENTS];	// 配列実体の確保
		}

		m_pArguments = new UnivType*[ARG_ARRAY_ELEMENTS * nBlock];	// ポインタ配列の確保
		for(i = 0, k = 0; i < nBlock; i++){
			for(j = 0; j < ARG_ARRAY_ELEMENTS; j++, k++){
				m_pArguments[k] = &m_pArgArray[i][j];
			}
		}
	}

	invalidate();
}

void Equation::setNumArguments(int n)
{
	if(isVariableArgumentNum() || (m_nArguments != n)){
		clearArguments();
	}

	if((m_nArguments != n) && (n > 0)){
		m_nArguments = n;

		m_pArguments = new UnivType*[m_nArguments];	// ポインタ配列の確保
		m_pArguments[0] = new UnivType[m_nArguments];	// 配列実体の確保
		for(int i = 1; i < m_nArguments; i++){
			m_pArguments[i] = m_pArguments[0] + i;
		}
	}

	invalidate();
}

UnivType& Equation::getArgumentAt(int n)
{
	assert(m_pArguments != NULL);
	assert(n < m_nArguments);
	invalidate();	// 引数に変更が加わる可能性があるため、算出結果を無効にする
	return *m_pArguments[n];
}

const UnivType& Equation::getArgumentAt(int n) const
{
	assert(m_pArguments != NULL);
	assert(n < m_nArguments);
	return *m_pArguments[n];
}

UnivType& Equation::addSingleArgument()
{
	if(isFixedArgumentNum()){
		throw CdbException(CDB_FIXED_FUNC_ARG);
	}

	int nCurrentBlock = (m_nArguments + ARG_ARRAY_ELEMENTS / 2) / ARG_ARRAY_ELEMENTS;
	int nRequiredBlock = (m_nArguments + 1 + ARG_ARRAY_ELEMENTS / 2) / ARG_ARRAY_ELEMENTS;

	if(nCurrentBlock != nRequiredBlock){
		int i, j, k;

		assert((nCurrentBlock + 1) == nRequiredBlock);
		UnivType **pNewArgs = new UnivType*[nRequiredBlock];
		for(i = 0; i < nCurrentBlock; i++){
			pNewArgs[i] = m_pArgArray[i];
		}
		pNewArgs[i] = new UnivType[ARG_ARRAY_ELEMENTS];
		delete[] m_pArgArray;
		m_pArgArray = pNewArgs;

		delete[] m_pArguments;
		m_pArguments = new UnivType*[ARG_ARRAY_ELEMENTS * nRequiredBlock];
		for(i = 0, k = 0; i < nRequiredBlock; i++){
			for(j = 0; j < ARG_ARRAY_ELEMENTS; j++, k++){
				m_pArguments[k] = &m_pArgArray[i][j];
			}
		}
	}

	// 要素数を１つ増やす
	m_nArguments++;

	// 新規追加要素を返す
	return *m_pArguments[m_nArguments - 1];
}

bool Equation::receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	if(confirmOperation(OP_CUSTOM_FUNC)){
		return false;	// 評価は行わない(再帰呼び出しの際の無限ループを避けるため)
		// 【注意】TODO : カスタム関数中でinvalidate()できなくても問題ないのか確認すること→修正が必要では？
	}

	bool changed = false;
	bool invalid = false;
	for(int i = 0; i < m_nArguments; i++){
		if(m_pArguments[i]->receiveNotifyUpdated(pUnivType, kind)){
			changed = true;
			// 引数の変化に結果が影響されるのかを判定する
			unsigned int flag = (1U << i);
			bool depend_valu;
			bool depend_size;
			if(m_eOperation < EFM_ID_BEGIN){
				assert(g_FuncDependList[m_eOperation].m_id == m_eOperation);
				depend_valu = (g_FuncDependList[m_eOperation].m_DependArgsValue & flag) != 0;
				depend_size = (g_FuncDependList[m_eOperation].m_DependArgsSize & flag) != 0;
			} else {
				depend_valu = (g_EmbeddedFuncMap[m_eOperation - EFM_ID_BEGIN].m_DependArgsValue & flag) != 0;
				depend_size = (g_EmbeddedFuncMap[m_eOperation - EFM_ID_BEGIN].m_DependArgsSize & flag) != 0;
			}
			if(((kind & UK_FLAG_VALUE) && depend_valu) || ((kind & UK_FLAG_ARRAY_SIZE) && depend_size)){
				invalid = true;
			}
		}
	}
	if(invalid){
		invalidate();
	}
	return changed;
}

bool Equation::replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith)
{
	// 【注意】TODO : OP_CUSTOM_FUNCにおける再帰呼び出しの際に無限ループに陥らないことを確認すること

	bool changed = false;
	for(int i = 0; i < m_nArguments; i++){
		if(m_pArguments[i]->replaceReference(pFindWhat, pReplaceWith)){
			changed = true;
		}
	}
	if(changed){
		invalidate();
	}
	return changed;
}

bool Equation::checkValidArguments() const
{
	for(int i = 0; i < m_nArguments; i++){
		m_pArguments[i]->update();
		if(!m_pArguments[i]->isValid()) return false;
	}
	return true;
}

// 【注意】この関数の用途は特にないものと思われる 
bool Equation::isConstant() const
{
	bool isConst = true;
	for(int i = 0; i < m_nArguments; i++){
		if(!m_pArguments[i]->isConstant()) isConst = false;
	}
	return isConst;
}

bool Equation::isFixed() const
{
	if(confirmOperation(OP_CUSTOM_FUNC)){
		return false;
		// 評価は行わない(再帰呼び出しの際の無限ループを避けるため)
		// 【注意】TODO : カスタム関数中でinvalidate()できなくても問題ないのか確認すること→修正が必要では？
	}

	bool isFixed = true;
	for(int i = 0; i < m_nArguments; i++){
		if(!m_pArguments[i]->isFixed()) isFixed = false;
	}
	return isFixed;
}

void Equation::simplify() const
{
	if(confirmOperation(OP_CUSTOM_FUNC)){
		return;
		// 評価は行わない(再帰呼び出しの際の無限ループを避けるため)
		// 【注意】TODO : カスタム関数中でinvalidate()できなくても問題ないのか確認すること→修正が必要では？
	}

	for(int i = 0; i < m_nArguments; i++){
		m_pArguments[i]->simplify();
	}
}

bool Equation::isValid() const
{
	if(confirmOperation(OP_CUSTOM_FUNC)){
		return true;	// 評価は行わない(再帰呼び出しの際の無限ループを避けるため)
	}

	if(confirmOperation(OP_INVALID)){
		return false;
	}
	return checkValidArguments();
}

void Equation::showErrorMessage()
{
	switch(m_eOperation){
	case OP_SUBSET_ACCESS:
		if(!m_pArguments[0]->isValid()){
			throw CdbException(CDB_NOT_SUBSET);
		}
		break;
	case OP_CUSTOM_FUNC_CALL:
		{
			assert(m_nArguments >= 1);
			// 最初の引数はユーザー定義関数
			UnivType& CustomFunc = *m_pArguments[0];
			// ユーザー定義関数であるか確認する
			if(! (CustomFunc.isEquation() && CustomFunc.getConstEquation()->confirmOperation(Equation::OP_CUSTOM_FUNC))){
				throw CdbException(CDB_NOT_FUNCTION1, static_cast<const NamedUnivType *>(CustomFunc.getReferenceRoot(false))->getName());
			}
			for(int i = 1; i < m_nArguments; i++){
				if(!m_pArguments[i]->isValid()){
					throw CdbException(CDB_CFUNC_INVALID_ARG3, 
						i,
						getOrdinalNumberSuffix(i),
						static_cast<const NamedUnivType *>(CustomFunc.getReferenceRoot(false))->getName()
						);
				}
			}
		}
		break;
	default:
		// 組み込み関数を呼ぶ
		if(m_eOperation >= EFM_ID_BEGIN){
			for(int i = 0; i < m_nArguments; i++){
				if(!m_pArguments[i]->isValid()){
					throw CdbException(CDB_CFUNC_INVALID_ARG3, 
						i + 1,
						getOrdinalNumberSuffix(i + 1),
						g_EmbeddedFuncMap[m_eOperation - EFM_ID_BEGIN].m_pName
						);
				}
			}
		}
	}
}

void Equation::calculate()
{
	// OP_CUSTOM_FUNCである場合には、引数にはリンク先が未定義の数式が入っているため評価しない
	if(confirmOperation(OP_CUSTOM_FUNC)){
//		m_CachedResult.clear();
		m_CachedResult.setString("Custom Function");	// 090802 カスタム関数である場合は、評価結果にCustom Functionという文字列を入れる
		return;
	}

	// 引数が全て有効であるかチェックする
	if(!checkValidArguments()){
		m_CachedResult.invalidate();
		showErrorMessage();
		return;
	}

	int i;
#if 1	// 2010/04/10 インデックス付きreferenceの場合にフル参照が張られるため
	// 算出結果にフル参照が設定されている場合には、解除する
	if(m_CachedResult.isReference()){
		m_CachedResult.releaseReference();
	}else if(m_CachedResult.isMatrix()){
		int n = m_CachedResult.getNumElements();
		for(i = 0; i < n; i++){
			m_CachedResult.getAt(i).releaseReference();
		}
	}
#endif

	UnivType temp;
	switch(m_eOperation){
	case OP_NONE:
		// 何もしない
		break;
	case OP_PLUS:
		assert(m_nArguments >= 2);
		AopPlus(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		for(i = 2; i < m_nArguments; i++){
			temp.move(m_CachedResult);
			AopPlus(m_CachedResult, temp, *m_pArguments[i]);
		}
		break;
	case OP_MULT:
		assert(m_nArguments >= 2);
		AopMult(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		for(i = 2; i < m_nArguments; i++){
			temp.move(m_CachedResult);
			AopMult(m_CachedResult, temp, *m_pArguments[i]);
		}
		break;
	case OP_DOT_MULT:
		assert(m_nArguments >= 2);
		AopDotMult(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		for(i = 2; i < m_nArguments; i++){
			temp.move(m_CachedResult);
			AopDotMult(m_CachedResult, temp, *m_pArguments[i]);
		}
		break;
	case OP_BIT_AND:
		assert(m_nArguments >= 2);
		AopBitAnd(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		for(i = 2; i < m_nArguments; i++){
			temp.move(m_CachedResult);
			AopBitAnd(m_CachedResult, temp, *m_pArguments[i]);
		}
		break;
	case OP_BIT_OR:
		assert(m_nArguments >= 2);
		AopBitOr(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		for(i = 2; i < m_nArguments; i++){
			temp.move(m_CachedResult);
			AopBitOr(m_CachedResult, temp, *m_pArguments[i]);
		}
		break;
	case OP_LOGICAL_AND:
		assert(m_nArguments >= 2);
		AopLogicalAnd(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		for(i = 2; i < m_nArguments; i++){
			temp.move(m_CachedResult);
			AopLogicalAnd(m_CachedResult, temp, *m_pArguments[i]);
		}
		break;
	case OP_LOGICAL_OR:
		assert(m_nArguments >= 2);
		AopLogicalOr(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		for(i = 2; i < m_nArguments; i++){
			temp.move(m_CachedResult);
			AopLogicalOr(m_CachedResult, temp, *m_pArguments[i]);
		}
		break;
	case OP_CAT_ROW:
		assert(m_nArguments >= 2);
		AopCatRow(m_CachedResult, m_pArguments, m_nArguments, false);
		break;
	case OP_CAT_ROW_CONST:
		assert(m_nArguments >= 2);
		AopCatRow(m_CachedResult, m_pArguments, m_nArguments, true);
		break;
	case OP_CAT_COLUMN:
		assert(m_nArguments >= 2);
		AopCatCol(m_CachedResult, m_pArguments, m_nArguments, false);
		break;
	case OP_CAT_COLUMN_CONST:
		assert(m_nArguments >= 2);
		AopCatCol(m_CachedResult, m_pArguments, m_nArguments, true);
		break;
	case OP_CAT_PARTIAL_REF:
		assert(m_nArguments >= 1);
		AopCatPartialRef(m_CachedResult, m_pArguments, m_nArguments);
		break;
	case OP_SERIES:
		if(m_nArguments == 2){
			AopSeries(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		}else if(m_nArguments == 3){
			AopSeries(m_CachedResult, *m_pArguments[0], *m_pArguments[1], *m_pArguments[2]);
		}else{
			assert(false);
		}
		break;
	case OP_ARRAY_SIZE:
		assert(m_nArguments <= 3);
		if(m_nArguments == 1){
			AopArraySize(m_CachedResult, *m_pArguments[0]);
		}else if(m_nArguments == 2){
			AopArraySize(m_CachedResult, *m_pArguments[0], int(*m_pArguments[1]), false);
		}else if(m_nArguments == 3){
			AopArraySize(m_CachedResult, *m_pArguments[0], int(*m_pArguments[1]), bool(*m_pArguments[2]));
		}else{
			assert(false);
		}
		break;
	case OP_IF:
		assert(m_nArguments == 3);
		AopIf(m_CachedResult, *m_pArguments[0], *m_pArguments[1], *m_pArguments[2], false);
		break;
	case OP_IF_CONST:
		assert(m_nArguments == 3);
		AopIf(m_CachedResult, *m_pArguments[0], *m_pArguments[1], *m_pArguments[2], true);
		break;
	case OP_COND:
		assert(m_nArguments >= 2);
		AopCond(m_CachedResult, m_pArguments, m_nArguments, false);
		break;
	case OP_COND_CONST:
		assert(m_nArguments >= 2);
		AopCond(m_CachedResult, m_pArguments, m_nArguments, true);
		break;
	case OP_SWITCH:
		assert(m_nArguments >= 2);
		AopSwitch(m_CachedResult, m_pArguments, m_nArguments, false);
		break;
	case OP_SWITCH_CONST:
		assert(m_nArguments >= 2);
		AopSwitch(m_CachedResult, m_pArguments, m_nArguments, true);
		break;
	case OP_MINUS:
		assert(m_nArguments == 2);
		AopMinus(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_DIV:
		assert(m_nArguments == 2);
		AopDiv(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_DOT_DIV:
		assert(m_nArguments == 2);
		AopDotDiv(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_POWER:
		assert(m_nArguments == 2);
		AopPower(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_DOT_POWER:
		assert(m_nArguments == 2);
		AopDotPower(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_BIT_XOR:
		assert(m_nArguments == 2);
		AopBitXor(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_LOGICAL_XOR:
		assert(m_nArguments == 2);
		AopLogicalXor(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_EQUAL:
		assert(m_nArguments == 2);
		AopEqual(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_NEQUAL:
		assert(m_nArguments == 2);
		AopNotEqual(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_LARGE:
		assert(m_nArguments == 2);
		AopLarge(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_SMALL:
		assert(m_nArguments == 2);
		AopSmall(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_ELARGE:
		assert(m_nArguments == 2);
		AopELarge(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_ESMALL:
		assert(m_nArguments == 2);
		AopESmall(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_MOD:
		assert(m_nArguments == 2);
		AopMod(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_UNARY_MINUS:
		assert(m_nArguments == 1);
		AopUnaryMinus(m_CachedResult, *m_pArguments[0]);
		break;
	case OP_RECIPROCAL:
		assert(false);
		// not implemented yet
		break;
	case OP_BIT_NOT:
		assert(m_nArguments == 1);
		AopBitNot(m_CachedResult, *m_pArguments[0]);
		break;
	case OP_LOGICAL_NOT:
		assert(m_nArguments == 1);
		AopLogicalNot(m_CachedResult, *m_pArguments[0]);
		break;
	case OP_CONJ:
		assert(m_nArguments == 1);
		AopComplexConj(m_CachedResult, *m_pArguments[0]);
		break;
	case OP_THROUGH_TILL_WRITE:	// 最初に書き込まれるまで第一引数をそのまま結果とする
		assert(m_nArguments == 1);
		m_CachedResult.evalCopy(*m_pArguments[0]);
		setFixCacheWhenWritten(true);
		break;
	case OP_SUBSET_ACCESS:
		assert(m_nArguments == 2);
		AopSubsetAccess(m_CachedResult, *m_pArguments[0], *m_pArguments[1]);
		break;
	case OP_CUSTOM_FUNC_CALL:
		assert(m_nArguments >= 1);
		// 最初の引数はユーザー定義関数の本体
		CustomFunctionCall(m_CachedResult, *m_pArguments[0], &m_pArguments[1], m_nArguments - 1);
		break;
	default:
		// 組み込み関数を呼ぶ
		if(m_eOperation >= EFM_ID_BEGIN){
			g_EmbeddedFuncMap[m_eOperation - EFM_ID_BEGIN].m_pfnFunc(m_CachedResult, (const UnivType **)m_pArguments, m_nArguments);
		}
	}

	// 書き込みイベントを検出するためのフックを設定
	if(m_FixCacheWhenWritten){
		setWriteEventHook();
	}

	validate();	// 算出結果の更新完了
}

void Equation::CustomFunctionCall(UnivType& result, UnivType& CustomFunc, UnivType** pFuncArgs, int nFuncArgs)
{
	int i;

	// ユーザー定義関数であるか確認する
	if(! (CustomFunc.isEquation() && CustomFunc.getConstEquation()->confirmOperation(Equation::OP_CUSTOM_FUNC))){
		throw CdbException(CDB_NOT_FUNCTION1, static_cast<const NamedUnivType *>(CustomFunc.getReferenceRoot(false))->getName());
	}

	const Equation *pEquation = CustomFunc.getConstEquation();
	int nArgs = pEquation->getNumArguments() - 1;	// 関数の引数の数
	if(nFuncArgs != nArgs){
		// 積まれた引数の数が異なる
		throw CdbException(CDB_CFUNC_INVALID_NARG2, nFuncArgs, nArgs);
	}
	UnivType& ReturnVal = *(pEquation->m_pArguments[0]);

	// 関数実体の仮引数の現時点でのバインド先を保存する
	const UnivType **pPrevBindedArgs = new const UnivType*[nArgs];
	for(i = 0; i < nArgs; i++){
		if(pEquation->m_pArguments[i + 1]->isReference()){
			IndexConv *pIndexConv;
			pEquation->m_pArguments[i + 1]->getReference(pPrevBindedArgs[i], pIndexConv);
		}else{
			pPrevBindedArgs[i] = NULL;
		}
	}
	// 実引数を入れる変数の確保
	UnivType *pArgs = new UnivType[nArgs];
	// 実引数の評価式を評価して実引数を得る
	for(i = 0; i < nArgs; i++){
		pArgs[i].evalCopy(*pFuncArgs[i]);
	}
	// 実引数を仮引数にバインドする
	for(i = 0; i < nArgs; i++){
		pEquation->m_pArguments[i + 1]->setConstReference(&pArgs[i]);
	}
	// 引数が切り替わったことを通知する
	for(i = 0; i < nArgs; i++){
		ReturnVal.receiveNotifyUpdated(pEquation->m_pArguments[i + 1], UK_ALL);	// 【注意】ここで更新は&pArgs[i]ではないことに注意。仮引数の部分に対して更新される。
	}
	// 結果をキャッシュにコピーする
	result.copy(ReturnVal);

#if 0	// キャッシュに参照を書くタイプの関数だと、再帰関数の際に無限ループに陥るため駄目
	if(ReturnVal.isEquation()){
		// 関数を評価して、結果をキャッシュに移動する
		Equation *pReturnEq = ReturnVal.getEquation();
		result.move(pReturnEq->eval());
	}else{
		// コピーする
		result.copy(ReturnVal);
	}
#endif

	// 実引数へのバインドを元に戻す
	for(i = 0; i < nArgs; i++){
		if(pPrevBindedArgs[i] != NULL){
			pEquation->m_pArguments[i + 1]->setConstReference(pPrevBindedArgs[i]);
		}else{
			pEquation->m_pArguments[i + 1]->releaseReference();
		}
	}
	// 引数が切り替わったことを通知する
	for(i = 0; i < nArgs; i++){
		ReturnVal.receiveNotifyUpdated(pEquation->m_pArguments[i + 1], UK_ALL);	// 【注意】ここで更新は&pPrevBindedArgs[i]ではないことに注意。仮引数の部分に対して更新される。
	}
	// 実引数を開放する
	delete[] pArgs;
	delete[] pPrevBindedArgs;
}

enum Equation::EqnOperation Equation::findEmbeddedFuncByName(const char *pName)
{
	for(int i = 0; g_EmbeddedFuncMap[i].m_pName; i++){
		if(strcmp(pName, g_EmbeddedFuncMap[i].m_pName) == 0){
			return (enum EqnOperation)(g_EmbeddedFuncMap[i].m_id + EFM_ID_BEGIN);
		}
	}
	return OP_INVALID;
}

bool Equation::isRequireHiddenArguments(enum EqnOperation eOp)
{
	return eOp == EFM_ID_BEGIN;
}

void Equation::clearCustomFuncArgs()
{
	// 全ての引数をクリアして、定数属性としておく
	for(int i = 1; i < m_nArguments; i++){
		m_pArguments[i]->clearAll();
		m_pArguments[i]->setConstant(true);
	}
}

// キャッシュ内容を固定し、全ての引数を削除する
void Equation::fixCache()
{
	// 要素間がリンクである可能性を考え、全ての内容を評価コピーする
	UnivType temp;
	temp.move(m_CachedResult);
	m_CachedResult.evalCopy(temp);
	temp.clearAll();
	validate();

	// モードを動作なしに設定
	setOperation(OP_NONE);

	// 引数を全てクリアする
	clearArguments();
}

void Equation::setWriteEventHook()
{
	if(m_CachedResult.isMatrix()){
		int nRow = m_CachedResult.getNumRow();
		int nCol = m_CachedResult.getNumColumn();
		int i, j;
		for(i = 0; i < nRow; i++){
			for(j = 0; j < nCol; j++){
				static_cast<OwnedUnivType *>(&m_CachedResult.getAt(i, j))->setOwner(this);
			}
		}
	}else{
		m_CachedResult.setOwner(this);
	}
}

void Equation::clearWriteEventHook()
{
	if(m_CachedResult.isMatrix()){
		int nRow = m_CachedResult.getNumRow();
		int nCol = m_CachedResult.getNumColumn();
		int i, j;
		for(i = 0; i < nRow; i++){
			for(j = 0; j < nCol; j++){
				static_cast<OwnedUnivType *>(&m_CachedResult.getAt(i, j))->setOwner(NULL);
			}
		}
	}else{
		m_CachedResult.setOwner(NULL);
	}
}

void Equation::copy(const Equation& src)
{
	clearArguments();

	m_eOperation = src.m_eOperation;
	m_CachedResult.copy(src.m_CachedResult);
	m_FixCacheWhenWritten = src.m_FixCacheWhenWritten;

	setNumArguments(src.m_nArguments);
	for(int i = 0; i < m_nArguments; i++){
		getArgumentAt(i).copy(src.getArgumentAt(i));
	}
}

#ifdef DEBUG_PRINT
#define TAB_INDENT(nIndentLevel) { for(int n = 0; n < nIndentLevel; n++) System::Console::Write("\t"); }
void Equation::debugPrintStructure(int nIndentLevel) const
{
	static const char *pOperationName[] = {
		"OP_CUSTOM_FUNC",
		"OP_CUSTOM_FUNC_CALL",
		"OP_PLUS",
		"OP_MULT",
		"OP_DOT_MULT",
		"OP_BIT_AND",
		"OP_BIT_OR",
		"OP_LOGICAL_AND",
		"OP_LOGICAL_OR",
		"OP_CAT_ROW",
		"OP_CAT_ROW_CONST",
		"OP_CAT_COLUMN",
		"OP_CAT_COLUMN_CONST",
		"OP_CAT_PARTIAL_REF",
		"OP_SERIES",
		"OP_ARRAY_SIZE",
		"OP_IF",
		"OP_IF_CONST",
		"OP_COND",
		"OP_COND_CONST",
		"OP_SWITCH",
		"OP_SWITCH_CONST",
		// binary operations
		"OP_MINUS",
		"OP_DIV",
		"OP_DOT_DIV",
		"OP_POWER",
		"OP_DOT_POWER",
		"OP_BIT_XOR",
		"OP_LOGICAL_XOR",
		"OP_EQUAL",
		"OP_NEQUAL",
		"OP_LARGE",
		"OP_SMALL",
		"OP_ELARGE",
		"OP_ESMALL",
		"OP_MOD",
		"OP_SUBSET_ACCESS",
		// unary operations
		"OP_UNARY_MINUS",
		"OP_RECIPROCAL",
		"OP_BIT_NOT",
		"OP_LOGICAL_NOT",
		"OP_CONJ",
		"OP_THROUGH_TILL_WRITE",
		// zero-ary operations
		"OP_NONE",
		// invalid operation
		"OP_INVALID"
	};
	System::String ^pOpName;
	if(m_eOperation >= EFM_ID_BEGIN){
		pOpName = "EMFUNC"+(m_eOperation - EFM_ID_BEGIN);
	}else{
		pOpName = gcnew System::String(pOperationName[(int)m_eOperation]);
	}
	System::Console::WriteLine("EQN:"+pOpName+"{");
	for(int i = 0; i < m_nArguments; i++){
		System::String ^pText = nullptr;
		if(m_eOperation == OP_CUSTOM_FUNC_CALL){
			if(i == 0){
				pText = "FUNC=";
			}else{
				pText = "ARG"+(i-1)+"=";
			}
		}else if(m_eOperation == OP_CUSTOM_FUNC){
			if(i == 0){
				pText = "DEF=";
			}else{
				pText = "ARG"+(i-1)+"=";
			}
		}else{
			pText = "ARG"+i+"=";
		}
		TAB_INDENT(nIndentLevel);
		System::Console::Write(pText);
		m_pArguments[i]->debugPrintStructure(nIndentLevel + 1);
		//if(i != (m_nArguments - 1)){
			System::Console::WriteLine(",");
		//}
	}
	TAB_INDENT(nIndentLevel);
	System::Console::Write("CACHE=");
	m_CachedResult.debugPrintStructure(nIndentLevel + 1);
	System::Console::Write("}");
}
#endif
