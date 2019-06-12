#include "UnivTypeOps.h"
#include "CdbException.h"
#include "NamedUnivType.h"
#include "CommonUtils.h"
#include "IndexConv.h"
#include "BaseDB.h"
#include <cassert>
#include <cstring>
#include <cmath>

// internal functions

static void ScalarPlus(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarMinus(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarMult(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarDiv(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarMod(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarBitAnd(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarBitOr(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarBitXor(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarBitNot(UnivType& result, const UnivType& x);
static void ScalarLogicalAnd(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarLogicalOr(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarLogicalXor(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarLogicalNot(UnivType& result, const UnivType& x);
static void ScalarEqual(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarNotEqual(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarLarge(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarSmall(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarELarge(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarESmall(UnivType& result, const UnivType& x, const UnivType& y);
static void ScalarUnaryMinus(UnivType& result, const UnivType& x);
static void ScalarComplexConj(UnivType& result, const UnivType& x);
static void ScalarPower(UnivType& result, const UnivType& x, const UnivType& y);

// C++ operator overload

UnivType operator+(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopPlus(result, x, y);
	return result;
}

UnivType operator-(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopMinus(result, x, y);
	return result;
}

UnivType operator*(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopMult(result, x, y);
	return result;
}

UnivType operator/(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopDiv(result, x, y);
	return result;
}

UnivType operator%(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopMod(result, x, y);
	return result;
}

UnivType operator&(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopBitAnd(result, x, y);
	return result;
}

UnivType operator|(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopBitOr(result, x, y);
	return result;
}

UnivType operator^(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopBitXor(result, x, y);
	return result;
}

UnivType operator&&(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopLogicalAnd(result, x, y);
	return result;
}

UnivType operator||(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopLogicalOr(result, x, y);
	return result;
}

UnivType operator!(const UnivType& x)
{
	UnivType result;
	AopLogicalNot(result, x);
	return result;
}

UnivType operator~(const UnivType& x)
{
	UnivType result;
	AopBitNot(result, x);
	return result;
}

UnivType operator==(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopEqual(result, x, y);
	return result;
}

UnivType operator!=(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopNotEqual(result, x, y);
	return result;
}

UnivType operator>(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopLarge(result, x, y);
	return result;
}

UnivType operator<(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopSmall(result, x, y);
	return result;
}

UnivType operator>=(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopELarge(result, x, y);
	return result;
}

UnivType operator<=(const UnivType& x, const UnivType& y)
{
	UnivType result;
	AopESmall(result, x, y);
	return result;
}

// internal functions

static void ScalarPlus(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x + (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x + (unsigned int)y;
	}else if(x.isReal()){
		result = (UnivType::Real)x + (UnivType::Real)y;
	}else if(x.isDouble()){
		result = (UnivType::Double)x + (UnivType::Double)y;
	}else if(x.isPointer()){
		unsigned char *p = (unsigned char *)x.getPointer();
		unsigned int n = (unsigned int)y;
		result.setPointer(p + n);
	}else if(x.isBoolean()){
		result = ((bool)x || (bool)y);
	}else if(x.isString()){
		UnivType z;
		y.toString(z);

		size_t len_x = x.getStringLength();
		size_t len_y = z.getStringLength();
		char *p = new char[len_x + len_y + 1];
		strcpy(p, x);
		strcat(p, z);
		result = p;
		delete[] p;
	}else if(x.isBinary()){
		UnivType z;
		y.toBinary(z);

		size_t size_x = x.getBinarySize();
		size_t size_y = z.getBinarySize();
		unsigned char *p = new unsigned char[size_x + size_y];
		memcpy(p, x.getBinary(), size_x);
		memcpy(p + size_x, z.getBinary(), size_y);
		result.setBinary(p, size_x + size_y);
		delete[] p;
	}else if(x.isSubset()){
		BaseDB *pDB = new BaseDB;	// subsetOwnerの設定の必要はない
		result.setSubset(pDB);
		pDB->connectDB(x.getSubsetDB());
		pDB->connectDB(y.getSubsetDB());
	}else{
		assert(false);
	}
}

static void ScalarMinus(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x - (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x - (unsigned int)y;
	}else if(x.isReal()){
		result = (UnivType::Real)x - (UnivType::Real)y;
	}else if(x.isDouble()){
		result = (UnivType::Double)x - (UnivType::Double)y;
	}else if(x.isPointer()){
		unsigned char *p = (unsigned char *)x.getPointer();
		unsigned int n = (unsigned int)y;
		result.setPointer(p - n);
	}else{
		assert(false);
	}
}

static void ScalarMult(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x * (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x * (unsigned int)y;
	}else if(x.isReal()){
		result = (UnivType::Real)x * (UnivType::Real)y;
	}else if(x.isDouble()){
		result = (UnivType::Double)x * (UnivType::Double)y;
	}else if(x.isBoolean()){
		result = (bool)x && (bool)y;
	}else{
		assert(false);
	}
}

static void ScalarDiv(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x / (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x / (unsigned int)y;
	}else if(x.isReal()){
		result = (UnivType::Real)x / (UnivType::Real)y;
	}else if(x.isDouble()){
		result = (UnivType::Double)x / (UnivType::Double)y;
	}else{
		assert(false);
	}
}

static void ScalarMod(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x % (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x % (unsigned int)y;
	}else{
		assert(false);
	}
}

static void ScalarBitAnd(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x & (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x & (unsigned int)y;
	}else if(x.isBoolean()){
		result = (bool)x && (bool)y;
	}else{
		assert(false);
	}
}

static void ScalarBitOr(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x | (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x | (unsigned int)y;
	}else if(x.isBoolean()){
		result = (bool)x || (bool)y;
	}else{
		assert(false);
	}
}

static void ScalarBitXor(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		result = (int)x ^ (int)y;
	}else if(x.isUInteger()){
		result = (unsigned int)x ^ (unsigned int)y;
	}else if(x.isBoolean()){
		result = (bool)x ^ (bool)y;
	}else{
		assert(false);
	}
}

static void ScalarBitNot(UnivType& result, const UnivType& x)
{
	if(x.isInteger()){
		result = ~(int)x;
	}else if(x.isUInteger()){
		result = ~(unsigned int)x;
	}else if(x.isBoolean()){
		result = !(bool)x;
	}else{
		assert(false);
	}
}

static void ScalarLogicalAnd(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger() || x.isUInteger() || x.isBoolean()){
		result = (bool)x && (bool)y;
	}else{
		assert(false);
	}
}

static void ScalarLogicalOr(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger() || x.isUInteger() || x.isBoolean()){
		result = (bool)x || (bool)y;
	}else{
		assert(false);
	}
}

static void ScalarLogicalXor(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger() || x.isUInteger() || x.isBoolean()){
		result = (bool)x ^ (bool)y;
	}else{
		assert(false);
	}
}

static void ScalarLogicalNot(UnivType& result, const UnivType& x)
{
	if(x.isInteger() || x.isUInteger() || x.isBoolean()){
		result = !(bool)x;
	}else{
		assert(false);
	}
}

static void ScalarEqual(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		if(y.isNumber()){
			if(y.isInteger() || y.isUInteger()){
				result = ((int)x == (int)y);
			}else if(y.isReal()){
				result = ((UnivType::Real)x == (UnivType::Real)y);
			}else if(y.isDouble()){
				result = ((UnivType::Double)x == (UnivType::Double)y);
			}
		}else{
			result = false;
		}
	}else if(x.isUInteger()){
		if(y.isNumber()){
			if(y.isInteger() || y.isUInteger()){
				result = ((unsigned int)x == (unsigned int)y);
			}else if(y.isReal()){
				result = ((UnivType::Real)x == (UnivType::Real)y);
			}else if(y.isDouble()){
				result = ((UnivType::Double)x == (UnivType::Double)y);
			}
		}else{
			result = false;
		}
	}else if(x.isReal()){
		if(y.isNumber()){
			result = ((UnivType::Real)x == (UnivType::Real)y);
		}else{
			result = false;
		}
	}else if(x.isDouble()){
		if(y.isNumber()){
			result = ((UnivType::Double)x == (UnivType::Double)y);
		}else{
			result = false;
		}
	}else if(x.isPointer()){
		if(y.isPointer()){
			result = (x.getPointer() == y.getPointer());
		}else{
			result = false;
		}
	}else if(x.isBoolean()){
		if(y.isBoolean()){
			result = ((bool)x == (bool)y);
		}else{
			result = false;
		}
	}else if(x.isString()){
		if(y.isString()){
			size_t len_x = x.getStringLength();
			size_t len_y = y.getStringLength();
			if(len_x == len_y){
				result = (strcmp(x, y) == 0);
			}else{
				result = false;
			}
		}else{
			result = false;
		}
	}else if(x.isBinary()){
		size_t size_x = x.getBinarySize();
		size_t size_y = y.getBinarySize();
		if(size_x == size_y){
			unsigned char *p1 = (unsigned char *)x.getBinary();
			unsigned char *p2 = (unsigned char *)y.getBinary();
			result = true;
			for(size_t i = 0; i < size_x; i++){
				if(*p1++ != *p2++){
					result = false;
				}
			}
		}else{
			result = false;
		}
	}else if(x.isEmpty()){
		result = y.isEmpty();
	}else{
		assert(false);
	}
}

static void ScalarNotEqual(UnivType& result, const UnivType& x, const UnivType& y)
{
	UnivType temp;
	ScalarEqual(temp, x, y);
	result = !(bool)temp;
}

static void ScalarLarge(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		if(y.isNumber()){
			if(y.isInteger() || y.isUInteger()){
				result = ((int)x > (int)y);
			}else if(y.isReal()){
				result = ((UnivType::Real)x > (UnivType::Real)y);
			}else if(y.isDouble()){
				result = ((UnivType::Double)x > (UnivType::Double)y);
			}
		}else{
			result = false;
		}
	}else if(x.isUInteger()){
		if(y.isNumber()){
			if(y.isInteger() || y.isUInteger()){
				result = ((unsigned int)x > (unsigned int)y);
			}else if(y.isReal()){
				result = ((UnivType::Real)x > (UnivType::Real)y);
			}else if(y.isDouble()){
				result = ((UnivType::Double)x > (UnivType::Double)y);
			}
		}else{
			result = false;
		}
	}else if(x.isReal()){
		if(y.isNumber()){
			result = ((UnivType::Real)x > (UnivType::Real)y);
		}else{
			result = false;
		}
	}else if(x.isDouble()){
		if(y.isNumber()){
			result = ((UnivType::Double)x > (UnivType::Double)y);
		}else{
			result = false;
		}
	}else if(x.isPointer()){
		if(y.isPointer()){
			result = (x.getPointer() > y.getPointer());
		}else{
			result = false;
		}
	}else if(x.isBoolean()){
		if(y.isBoolean()){
			result = ((bool)x && !(bool)y);
		}else{
			result = false;
		}
	}else if(x.isString()){
		if(y.isString()){
			size_t len_x = x.getStringLength();
			size_t len_y = y.getStringLength();
			if(len_x == len_y){
				result = (strcmp(x, y) > 0);
			}else{
				result = false;
			}
		}else{
			result = false;
		}
	}else if(x.isBinary()){
		throw CdbException(CDB_CANNOT_COMPARE, "Binary");
	}else if(x.isEmpty()){
		result = y.isEmpty();
	}else{
		assert(false);
	}
}

static void ScalarSmall(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isInteger()){
		if(y.isNumber()){
			if(y.isInteger() || y.isUInteger()){
				result = ((int)x < (int)y);
			}else if(y.isReal()){
				result = ((UnivType::Real)x < (UnivType::Real)y);
			}else if(y.isDouble()){
				result = ((UnivType::Double)x < (UnivType::Double)y);
			}
		}else{
			result = false;
		}
	}else if(x.isUInteger()){
		if(y.isNumber()){
			if(y.isInteger() || y.isUInteger()){
				result = ((unsigned int)x < (unsigned int)y);
			}else if(y.isReal()){
				result = ((UnivType::Real)x < (UnivType::Real)y);
			}else if(y.isDouble()){
				result = ((UnivType::Double)x < (UnivType::Double)y);
			}
		}else{
			result = false;
		}
	}else if(x.isReal()){
		if(y.isNumber()){
			result = ((UnivType::Real)x < (UnivType::Real)y);
		}else{
			result = false;
		}
	}else if(x.isDouble()){
		if(y.isNumber()){
			result = ((UnivType::Double)x < (UnivType::Double)y);
		}else{
			result = false;
		}
	}else if(x.isPointer()){
		if(y.isPointer()){
			result = (x.getPointer() < y.getPointer());
		}else{
			result = false;
		}
	}else if(x.isBoolean()){
		if(y.isBoolean()){
			result = (!(bool)x && (bool)y);
		}else{
			result = false;
		}
	}else if(x.isString()){
		if(y.isString()){
			size_t len_x = x.getStringLength();
			size_t len_y = y.getStringLength();
			if(len_x == len_y){
				result = (strcmp(x, y) < 0);
			}else{
				result = false;
			}
		}else{
			result = false;
		}
	}else if(x.isBinary()){
		throw CdbException(CDB_CANNOT_COMPARE, "Binary");
	}else if(x.isEmpty()){
		result = y.isEmpty();
	}else{
		assert(false);
	}
}

static void ScalarELarge(UnivType& result, const UnivType& x, const UnivType& y)
{
	ScalarLarge(result, x, y);
	if(!(bool)result){
		ScalarEqual(result, x, y);
	}
}

static void ScalarESmall(UnivType& result, const UnivType& x, const UnivType& y)
{
	ScalarSmall(result, x, y);
	if(!(bool)result){
		ScalarEqual(result, x, y);
	}
}

static void ScalarUnaryMinus(UnivType& result, const UnivType& x)
{
	if(x.isInteger()){
		result = -(int)x;
	}else if(x.isReal()){
		result = -(UnivType::Real)x;
	}else{
		assert(false);
	}
}

static void ScalarComplexConj(UnivType& result, const UnivType& x)
{
	// 未実装
	// TODO : 今は参照設定となっているが、ここは代入式で実装すること
	result.setConstReference(&x);
}

static void ScalarPower(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(!y.isNumber()){
		throw CdbException(CDB_CFUNC_INVALID_ARG2, "ScalarPower", "The second argument should be number");
	}

	if(x.isInteger()){
		if(y.isInteger() || y.isUInteger()){
			result = (int)pow((UnivType::Real)x, (int)y);
		}else{
			result = (int)pow((UnivType::Real)x, (UnivType::Real)y);
		}
	}else if(x.isUInteger()){
		if(y.isInteger() || y.isUInteger()){
			result = (unsigned int)pow((UnivType::Real)x, (int)y);
		}else{
			result = (unsigned int)pow((UnivType::Real)x, (UnivType::Real)y);
		}
	}else if(x.isReal()){
		if(y.isInteger() || y.isUInteger()){
			result = (UnivType::Real)pow((UnivType::Real)x, (int)y);
		}else{
			result = (UnivType::Real)pow((UnivType::Real)x, (UnivType::Real)y);
		}
	}else if(x.isDouble()){
		if(y.isInteger() || y.isUInteger()){
			result = (UnivType::Double)pow((UnivType::Double)x, (int)y);
		}else{
			result = (UnivType::Double)pow((UnivType::Double)x, (UnivType::Double)y);
		}
	}else{
		throw CdbException(CDB_CFUNC_INVALID_ARG2, "ScalarPower", "The first argument should be number");
	}

}

// Common utilities ------------------------------

void ElementOperationScalarAndMatrix(UnivType& result, const UnivType& x, const UnivType& y, ScalarBinaryOperations ScalarOp)
{
	if(x.isMatrix() && y.isMatrix()){
		if(x.getNumColumn() != y.getNumColumn()){
			throw CdbException(CDB_MATRIX_NOT_CONFORM3, "column", x.getNumColumn(), y.getNumColumn());
		}
		if(x.getNumRow() != y.getNumRow()){
			throw CdbException(CDB_MATRIX_NOT_CONFORM3, "row", x.getNumRow(), y.getNumRow());
		}
		result.setMatrix(x.getNumRow(), x.getNumColumn());
		int n = x.getNumElements();
		for(int i = 0; i < n; i++){
			ScalarOp(result[i], x[i], y[i]);
		}
	}else if(x.isScalar() && y.isMatrix()){
		result.setMatrix(y.getNumRow(), y.getNumColumn());
		int n = y.getNumElements();
		for(int i = 0; i < n; i++){
			ScalarOp(result[i], x, y[i]);
		}
	}else if(x.isMatrix() && y.isScalar()){
		result.setMatrix(x.getNumRow(), x.getNumColumn());
		int n = x.getNumElements();
		for(int i = 0; i < n; i++){
			ScalarOp(result[i], x[i], y);
		}
	}else if(x.isScalar() && y.isScalar()){
		ScalarOp(result, x, y);
	}else{
		assert(false);
	}
}

void ElementOperationUnaryMatrix(UnivType& result, const UnivType& x, ScalarUnaryOperations ScalarOp)
{
	if(x.isScalar()){
		ScalarOp(result, x);
	}else if(x.isMatrix()){
		int n = x.getNumElements();
		result.setMatrix(x.getNumRow(), x.getNumColumn());
		for(int i = 0; i < n; i++){
			if(x[i].isScalar()){
				ScalarOp(result[i], x[i]);
			}else{
				ElementOperationUnaryMatrix(result[i], x[i], ScalarOp);
			}
		}
	}else{
		assert(false);
	}
}

// Arithmetic operations

void AopPlus(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarPlus);
}

void AopMinus(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarMinus);
}

void AopMult(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(x.isScalar() || y.isScalar()){
		ElementOperationScalarAndMatrix(result, x, y, ScalarMult);
	}else if(x.isMatrix() && y.isMatrix()){
		if(x.getNumColumn() != y.getNumRow()){
			throw CdbException(CDB_MATRIX_NOT_CONFORM2, x.getNumColumn(), y.getNumRow());
		}
		int nRow = x.getNumRow();
		int nCol = y.getNumColumn();
		if((nRow == 1) && (nCol == 1)){
			// result is scalar
			int n = x.getNumColumn();
			ScalarMult(result, x.getAt(0, 0), y.getAt(0, 0));
			for(int i = 1; i < n; i++){
				UnivType sum;
				sum.move(result);
				UnivType temp;
				ScalarMult(temp, x.getAt(0, i), y.getAt(i, 0));
				ScalarPlus(result, sum, temp);
			}
		}else{
			// result is matrix
			result.setMatrix(nRow, nCol);
			int n = x.getNumColumn();
			for(int k = 0; k < nCol; k++){
				for(int j = 0; j < nRow; j++){
					UnivType& scalar_result = result.getAt(j, k);
					ScalarMult(scalar_result, x.getAt(j, 0), y.getAt(0, k));
					for(int i = 1; i < n; i++){
						UnivType sum;
						sum.move(scalar_result);
						UnivType temp;
						ScalarMult(temp, x.getAt(j, i), y.getAt(i, k));
						ScalarPlus(scalar_result, sum, temp);
					}
				}
			}
		}
	}else{
		assert(false);
	}
}

void AopDiv(UnivType& result, const UnivType& x, const UnivType& y)
{
	// とりあえずAopDotDivと同じ動作とする
	// 今後、逆行列に変更する可能性もある
	ElementOperationScalarAndMatrix(result, x, y, ScalarDiv);
}

void AopMod(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarMod);
}

void AopDotMult(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarMult);
}

void AopDotDiv(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarDiv);
}

void AopBitAnd(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarBitAnd);
}

void AopBitOr(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarBitOr);
}

void AopBitXor(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarBitXor);
}

void AopBitNot(UnivType& result, const UnivType& x)
{
	ElementOperationUnaryMatrix(result, x, ScalarBitNot);
}

void AopLogicalAnd(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarLogicalAnd);
}

void AopLogicalOr(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarLogicalOr);
}

void AopLogicalXor(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarLogicalXor);
}

void AopLogicalNot(UnivType& result, const UnivType& x)
{
	ElementOperationUnaryMatrix(result, x, ScalarLogicalNot);
}

void AopEqual(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarEqual);
}

void AopNotEqual(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarNotEqual);
}

void AopLarge(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarLarge);
}

void AopSmall(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarSmall);
}

void AopELarge(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarELarge);
}

void AopESmall(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarESmall);
}

void AopPower(UnivType& result, const UnivType& x, const UnivType& y)
{
	if(y.isInteger() || y.isReal()){
		if(x.isScalar()){
			ScalarPower(result, x, y);
		}else{
			UnivType::Real real_n = (UnivType::Real)y;
			int n = static_cast<int>(real_n);
			if((real_n != n) || (n < 0)){
				// 小数に対しての行列に対するpowerは未実装 
				throw CdbException(CDB_NOT_IMPLEMENTED);
			}else{
				if(x.getNumColumn() != x.getNumRow()){
					throw CdbException(CDB_CFUNC_INVALID_ARG2, "AopPower", "The first argument should be square-matrix");
				}
				int i, j, m;
				m = x.getNumRow();
				result.setMatrix(m, m);
				for(i = 0; i < m; i++){
					for(j = 0; j < m; j++){
						result.getAt(i, j) = (i == j) ? 1 : 0;	// 単位行列を入れておく
					}
				}
				UnivType temp;
				for(i = 0; i < n; i++){
					temp.evalCopy(result);
					AopMult(result, x, temp);	// tempの最初の型がどうであれ、resultにはxと同じ型が入る
				}
			}
		}
	}else{
		throw CdbException(CDB_CFUNC_INVALID_ARG2, "AopPower", "The second argument should be number");
	}
}

void AopDotPower(UnivType& result, const UnivType& x, const UnivType& y)
{
	ElementOperationScalarAndMatrix(result, x, y, ScalarPower);
}

void AopUnaryMinus(UnivType& result, const UnivType& x)
{
	ElementOperationUnaryMatrix(result, x, ScalarUnaryMinus);
}

void AopComplexConj(UnivType& result, const UnivType& x)
{
	ElementOperationUnaryMatrix(result, x, ScalarComplexConj);
}

void AopSeries(UnivType& result, const UnivType& begin, const UnivType& end)
{
	int nBegin = (int)begin;
	int nEnd = (int)end;
	AopSeries(result, begin, (nBegin < nEnd) ? 1 : (-1), end);
}

void AopSeries(UnivType& result, const UnivType& begin, const UnivType& delta, const UnivType& end)
{
	int nBegin = (int)begin;
	int nDelta = (int)delta;
	int nEnd = (int)end;

	if(nBegin == nEnd){
		result = nBegin;
	}else{
		if(nDelta == 0) throw CdbException(CDB_SERIES_INVALID_PARAM, (int)begin, (int)delta, (int)end);
		int n = (nEnd - nBegin) / nDelta;
		if(n < 0) throw CdbException(CDB_SERIES_INVALID_PARAM, (int)begin, (int)delta, (int)end);
		result.setMatrix(1, n + 1);
		for(int i = 0; i <= n; i++) result[i] = nBegin + nDelta * i;
	}
}

// branch operations

void AopIf(UnivType& result, const UnivType& cond, UnivType& arg0, UnivType& arg1, bool isConstRef)
{
	if((bool)cond){
		if(isConstRef){
			result.setConstReference(&arg0);
		}else{
			result.setFullReference(&arg0);
		}
	}else{
		if(isConstRef){
			result.setConstReference(&arg1);
		}else{
			result.setFullReference(&arg1);
		}
	}
}

void AopCond(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef)
{
	int cond = pArgs[0]->getInt();
	if((cond < 0) || (cond + 1 >= nArgs)) throw CdbException(CDB_COND_REF_OVER, cond);
	if(isConstRef){
		result.setConstReference(pArgs[cond + 1]);
	}else{
		result.setFullReference(pArgs[cond + 1]);
	}
}

void AopSwitch(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef)
{
	UnivType& val = *pArgs[0];
	UnivType& default_val = *pArgs[1];
	int n = (nArgs - 2) / 2;
	for(int i = 0; i < n; i++){
		int index = 2 + 2 * i;
		if(val == *pArgs[index]){
			if(isConstRef){
				result.setConstReference(pArgs[index + 1]);
			}else{
				result.setFullReference(pArgs[index + 1]);
			}
			return;
		}
	}

	if(isConstRef){
		result.setConstReference(&default_val);
	}else{
		result.setFullReference(&default_val);
	}
}

// array operations

// 引数
// *pArgs[2n + 0] : n番目の要素
// *pArgs[2n + 1] : n番目の要素のネスト設定フラグ = { true, false }
void AopCatRow(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef)
{
	assert(nArgs > 0);
	// 結果の行列サイズ決定
	int nRow, nColumn;
	if(pArgs[1]->getBool()){
		nRow = 1;
		nColumn = 1;
	}else{
		nRow = pArgs[0]->getNumRow();
		nColumn = pArgs[0]->getNumColumn();
	}
	int i, j, k, n;
	for(i = 2; i < nArgs; i += 2){
		int nElementRow, nElementCol;
		if(pArgs[i + 1]->getBool()){
			nElementRow = 1;
			nElementCol = 1;
		}else{
			nElementRow = pArgs[i]->getNumRow();
			nElementCol = pArgs[i]->getNumColumn();
		}
		if(nColumn != nElementCol){
			throw CdbException(CDB_CAT_DIFFERENT_DIM, "row", "column");
		}
		nRow += nElementRow;
	}

	// リンク生成
	result.setMatrix(nRow, nColumn);
	for(i = 0, n = 0; i < nArgs; i += 2){
		UnivType& element = *pArgs[i];
		UnivType& flag = *pArgs[i + 1];
		if(element.isScalar() || (bool)flag){
			if(isConstRef){
				result.getAt(n, 0).setConstReference(&element);
			}else{
				result.getAt(n, 0).setFullReference(&element);
			}
			n++;
		}else{
			for(j = 0; j < element.getNumRow(); j++, n++){
				for(k = 0; k < nColumn; k++){
					if(isConstRef){
						result.getAt(n, k).setConstReference(&element.getAtConst(j, k));
					}else{
						result.getAt(n, k).setFullReference(&element.getAt(j, k));
					}
				}
			}
		}
	}
}

// 引数
// *pArgs[2n + 0] : n番目の要素
// *pArgs[2n + 1] : n番目の要素のネスト設定フラグ = { true, false }
void AopCatCol(UnivType& result, UnivType** pArgs, int nArgs, bool isConstRef)
{
	assert(nArgs > 0);
	// 結果の行列サイズ決定
	int nRow, nColumn;
	if(pArgs[1]->getBool()){
		nRow = 1;
		nColumn = 1;
	}else{
		nRow = pArgs[0]->getNumRow();
		nColumn = pArgs[0]->getNumColumn();
	}
	int i, j, k, n;
	for(i = 2; i < nArgs; i += 2){
		int nElementRow, nElementCol;
		if(pArgs[i + 1]->getBool()){
			nElementRow = 1;
			nElementCol = 1;
		}else{
			nElementRow = pArgs[i]->getNumRow();
			nElementCol = pArgs[i]->getNumColumn();
		}
		if(nRow != nElementRow){
			throw CdbException(CDB_CAT_DIFFERENT_DIM, "column", "row");
		}
		nColumn += nElementCol;
	}

	// リンク生成
	result.setMatrix(nRow, nColumn);
	for(i = 0, n = 0; i < nArgs; i += 2){
		UnivType& element = *pArgs[i];
		UnivType& flag = *pArgs[i + 1];
		if(element.isScalar() || (bool)flag){
			if(isConstRef){
				result.getAt(0, n).setConstReference(&element);
			}else{
				result.getAt(0, n).setFullReference(&element);
			}
			n++;
		}else{
			for(j = 0; j < element.getNumColumn(); j++, n++){
				for(k = 0; k < nRow; k++){
					if(isConstRef){
						result.getAt(k, n).setConstReference(&element.getAtConst(k, j));
					}else{
						result.getAt(k, n).setFullReference(&element.getAt(k, j));
					}
				}
			}
		}
	}
}

void AopCatPartialRef(UnivType& result, UnivType** pArgs, int nArgs)
{
	int i, j, k;

	// 参照行列のサイズを決定
	int nRow = 0;
	int nCol = 0;
	int temp;
	assert(nArgs > 0);
	for(i = 0; i < nArgs; i++){
		const UnivType *pRef;
		IndexConv *pIndexConv;
		pArgs[i]->getReference(pRef, pIndexConv);
		assert(pIndexConv != NULL);

		// 各定義式より、参照行列の被参照領域を取得
		int begin_row, begin_col, size_row, size_col;
		pIndexConv->getReferencedArea(begin_row, begin_col, size_row, size_col);
		switch(begin_row){
		case INDEX_BEGIN:
			assert(size_row >= 0);
			temp = 0 + size_row;
			if(temp > nRow) nRow = temp;
			break;
		case INDEX_END:
			assert(size_row <= 0);
			temp = -size_row;
			if(temp > nRow) nRow = temp;
			break;
		case INDEX_INVALID:
			assert(false);
			break;
		default:
			temp = begin_row + size_row;
			assert(temp >= 0);
			if(temp > nRow) nRow = temp;
			break;
		}

		switch(begin_col){
		case INDEX_BEGIN:
			assert(size_col >= 0);
			temp = 0 + size_col;
			if(temp > nCol) nCol = temp;
			break;
		case INDEX_END:
			assert(size_col <= 0);
			temp = -size_col;
			if(temp > nCol) nCol = temp;
			break;
		case INDEX_INVALID:
			assert(false);
			break;
		default:
			temp = begin_col + size_col;
			assert(temp >= 0);
			if(temp > nCol) nCol = temp;
			break;
		}
	}

	// 行列を確保して、全要素を無効化しておく
	result.setMatrix(nRow, nCol);
	for(j = 0; j < nCol; j++){
		for(i = 0; i < nRow; i++){
			result.getAt(i, j).invalidate();
		}
	}

	for(k = 0; k < nArgs; k++){
		UnivType *pRef;
		IndexConv *pIndexConv;
		bool isConstRef = pArgs[k]->isConstReference();
		if(isConstRef){
			const UnivType *pConstRef;
			pArgs[k]->getReference(pConstRef, pIndexConv);
			pRef = const_cast<UnivType *>(pConstRef);
		}else{
			pArgs[k]->getReference(pRef, pIndexConv);
		}

		// 行列サイズの変更を通知
		pIndexConv->receiveNotifyUpdated(pRef, UK_ONLY_ARRAY_SIZE);

		// 要素間の参照関係を決定
		int size_row = pRef->getNumRow();
		int size_col = pRef->getNumColumn();
		for(j = 0; j < size_col; j++){
			for(i = 0; i < size_row; i++){
				UnivType& x = pArgs[k]->getAt(i, j);
				if(x.isValid()) throw CdbException(CDB_DEFINITION_NOT_UNIQUE);
				if(isConstRef){
					x.setConstReference(&pRef->getAt(i, j));
				}else{
					x.setFullReference(&pRef->getAt(i, j));
				}
			}
		}
	}
}

void AopArraySize(UnivType& result, UnivType& array)
{
	result.setMatrix(2, 1);
	result.getAt(0) = array.getNumRow();
	result.getAt(1) = array.getNumColumn();
}

void AopArraySize(UnivType& result, UnivType& array, int nDim, bool isVectorSize)
{
	switch(nDim){
	case DIM_ROW:
		if(isVectorSize){
			// ベクトルアクセスの際のサイズ
			result = array.getNumRow() * array.getNumColumn();
		}else{
			// 行列アクセスの際のサイズ
			result = array.getNumRow();
		}
		break;
	case DIM_COL:
		if(isVectorSize){
			// ベクトルアクセスの際のサイズ
			result = 1;
		}else{
			// 行列アクセスの際のサイズ
			result = array.getNumColumn();
		}
		break;
	default:
		assert(false);
	}
}

// subset operations

void AopSubsetAccess(UnivType& result, UnivType& subset, const char *pVariableName)
{
	if(!subset.isSubset()){
		throw CdbException(CDB_NOT_SUBSET);
	}

	BaseDB *pDB = subset.getSubsetDB();
	result.releaseReference();
	result.setFullReference(&pDB->getUnsafeUnivType(pVariableName), false);	// 「.」によるアクセスでは、指定されたサブセット以外の検索（サブセット所有者への検索）は行わない
}
