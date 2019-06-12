#ifndef DEF_UNIV_TYPE_OPS
#define DEF_UNIV_TYPE_OPS

// UnivType operations

#include "UnivType.h"

// C++ operator overload

UnivType operator+(const UnivType& x, const UnivType& y);
UnivType operator-(const UnivType& x, const UnivType& y);
UnivType operator*(const UnivType& x, const UnivType& y);
UnivType operator/(const UnivType& x, const UnivType& y);
UnivType operator%(const UnivType& x, const UnivType& y);
UnivType operator&(const UnivType& x, const UnivType& y);
UnivType operator|(const UnivType& x, const UnivType& y);
UnivType operator^(const UnivType& x, const UnivType& y);
UnivType operator&&(const UnivType& x, const UnivType& y);
UnivType operator||(const UnivType& x, const UnivType& y);
UnivType operator!(const UnivType& x);
UnivType operator~(const UnivType& x);
UnivType operator==(const UnivType& x, const UnivType& y);
UnivType operator!=(const UnivType& x, const UnivType& y);
UnivType operator>(const UnivType& x, const UnivType& y);
UnivType operator<(const UnivType& x, const UnivType& y);
UnivType operator>=(const UnivType& x, const UnivType& y);
UnivType operator<=(const UnivType& x, const UnivType& y);

// Arithmetic operations

void AopPlus(UnivType& result, const UnivType& x, const UnivType& y);
void AopMinus(UnivType& result, const UnivType& x, const UnivType& y);
void AopMult(UnivType& result, const UnivType& x, const UnivType& y);
void AopDiv(UnivType& result, const UnivType& x, const UnivType& y);
void AopMod(UnivType& result, const UnivType& x, const UnivType& y);
void AopDotMult(UnivType& result, const UnivType& x, const UnivType& y);
void AopDotDiv(UnivType& result, const UnivType& x, const UnivType& y);
void AopBitAnd(UnivType& result, const UnivType& x, const UnivType& y);
void AopBitOr(UnivType& result, const UnivType& x, const UnivType& y);
void AopBitXor(UnivType& result, const UnivType& x, const UnivType& y);
void AopBitNot(UnivType& result, const UnivType& x);
void AopLogicalAnd(UnivType& result, const UnivType& x, const UnivType& y);
void AopLogicalOr(UnivType& result, const UnivType& x, const UnivType& y);
void AopLogicalXor(UnivType& result, const UnivType& x, const UnivType& y);
void AopLogicalNot(UnivType& result, const UnivType& x);
void AopEqual(UnivType& result, const UnivType& x, const UnivType& y);
void AopNotEqual(UnivType& result, const UnivType& x, const UnivType& y);
void AopLarge(UnivType& result, const UnivType& x, const UnivType& y);
void AopSmall(UnivType& result, const UnivType& x, const UnivType& y);
void AopELarge(UnivType& result, const UnivType& x, const UnivType& y);
void AopESmall(UnivType& result, const UnivType& x, const UnivType& y);
void AopPower(UnivType& result, const UnivType& x, const UnivType& y);
void AopDotPower(UnivType& result, const UnivType& x, const UnivType& y);

void AopUnaryMinus(UnivType& result, const UnivType& x);
void AopComplexConj(UnivType& result, const UnivType& x);
void AopSeries(UnivType& result, const UnivType& begin, const UnivType& end);
void AopSeries(UnivType& result, const UnivType& begin, const UnivType& delta, const UnivType& end);

void AopCatRow(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef);
void AopCatCol(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef);
void AopCatPartialRef(UnivType& result, UnivType** pArgs, int nArgs);

void AopArraySize(UnivType& result, UnivType& array);
void AopArraySize(UnivType& result, UnivType& array, int nDim, bool isVectorSize);
void AopSubsetAccess(UnivType& result, UnivType& subset, const char *pVariableName);

void AopIf(UnivType& result, const UnivType& cond, UnivType& arg0, UnivType& arg1, bool isConstRef);
void AopCond(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef);
void AopSwitch(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef);

// some handy functions for operations of UnivType
typedef void (*ScalarBinaryOperations)(UnivType& result, const UnivType& x, const UnivType& y);
typedef void (*ScalarUnaryOperations)(UnivType& result, const UnivType& x);
void ElementOperationScalarAndMatrix(UnivType& result, const UnivType& x, const UnivType& y, ScalarBinaryOperations ScalarOp);
void ElementOperationUnaryMatrix(UnivType& result, const UnivType& x, ScalarUnaryOperations ScalarOp);

#endif
