#ifndef DEF_EQUATION
#define DEF_EQUATION

#include "UnivType.h"
#include "OwnedUnivType.h"
#include "RequireUpdateObj.h"

class Equation : public RequireUpdateObj
{
public:
	enum EqnOperation{
		// multi-ary operations
		OP_CUSTOM_FUNC = 0,
		OP_CUSTOM_FUNC_CALL,
		OP_PLUS,
		OP_MULT,
		OP_DOT_MULT,
		OP_BIT_AND,
		OP_BIT_OR,
		OP_LOGICAL_AND,
		OP_LOGICAL_OR,
		OP_CAT_ROW,
		OP_CAT_ROW_CONST,
		OP_CAT_COLUMN,
		OP_CAT_COLUMN_CONST,
		OP_CAT_PARTIAL_REF,
		OP_SERIES,
		OP_ARRAY_SIZE,
		OP_IF,
		OP_IF_CONST,
		OP_COND,
		OP_COND_CONST,
		OP_SWITCH,
		OP_SWITCH_CONST,
		// binary operations
		OP_MINUS,
		OP_DIV,
		OP_DOT_DIV,
		OP_POWER,
		OP_DOT_POWER,
		OP_BIT_XOR,
		OP_LOGICAL_XOR,
		OP_EQUAL,
		OP_NEQUAL,
		OP_LARGE,
		OP_SMALL,
		OP_ELARGE,
		OP_ESMALL,
		OP_MOD,
		OP_SUBSET_ACCESS,
		// unary operations
		OP_UNARY_MINUS,
		OP_RECIPROCAL,
		OP_BIT_NOT,
		OP_LOGICAL_NOT,
		OP_CONJ,
		OP_THROUGH_TILL_WRITE,
		// zero-ary operations
		OP_NONE,
		// invalid operation
		OP_INVALID
	};
	static struct FuncDependFlag Equation::g_FuncDependList[];

private:
	enum EqnOperation m_eOperation;
	OwnedUnivType m_CachedResult;
	bool m_FixCacheWhenWritten;	// 計算結果キャッシュに行列が入っていたときに、そのいづれかの要素に変更が加えられたらキャッシュ内容をfixする

	// 引数
	UnivType **m_pArguments;
	UnivType **m_pArgArray;
	int m_nArguments;

private:
	bool isFixedArgumentNum(){ return (m_pArguments != NULL) && (m_pArgArray == NULL); }	// 引数の数を固定(setNumArguments()でのみ設定可能)
	bool isVariableArgumentNum(){ return (m_pArguments != NULL) && (m_pArgArray != NULL); }	// 引数の数が可変(addSingleArgument()が利用可能)
	bool isArgumentEmpty(){ return (m_pArguments == NULL) && (m_pArgArray == NULL); }
	void setNumArgumentsVariable(int n);	// 可変個の引数を一度に確保する
	void clearArguments();

public:
	Equation();
	~Equation();

	UnivType& eval(){ if(requireUpdate()) calculate(); return m_CachedResult; }

	void setOperation(enum EqnOperation op){ m_eOperation = op; }
	void setCustomFuncOp(const char *pFuncName);
	void setNumArguments(int n);
	int getNumArguments() const { return m_nArguments; }
	UnivType& getArgumentAt(int n);
	const UnivType& getArgumentAt(int n) const;
	UnivType& operator[](int n){ return getArgumentAt(n); }
	const UnivType& operator[](int n) const { return getArgumentAt(n); }
	UnivType& addSingleArgument();
	bool confirmOperation(enum EqnOperation eOperation) const { return eOperation == m_eOperation; }

	virtual bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	bool replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith);

	bool isConstant() const;
	bool isFixed() const;
	bool isValid() const;
	void simplify() const;
	void copy(const Equation& src);

	void clearCustomFuncArgs();
	void showErrorMessage();

	static enum EqnOperation findEmbeddedFuncByName(const char *pName);
	static bool isRequireHiddenArguments(enum EqnOperation eOp);

	void setFixCacheWhenWritten(bool setting){ m_FixCacheWhenWritten = setting; }
	void fixCache();
	void setWriteEventHook();
	void clearWriteEventHook();

#ifdef DEBUG_PRINT
	void debugPrintStructure(int nIndentLevel) const;
#endif

private:
	void calculate();
	bool checkValidArguments() const;
	static void CustomFunctionCall(UnivType& result, UnivType& CustomFunc, UnivType** pArgs, int nArgs);

};

#endif
