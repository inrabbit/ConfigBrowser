#include "Serializer.h"
#include "BaseDB.h"
#include "UnivType.h"
#include "NamedUnivType.h"
#include "CdbException.h"

#include <fstream>
#include <iomanip>
#include <cassert>

using namespace std;

void WriteVectorItem(ofstream& ofstr, const UnivType *pUnivType);
void WriteScalarItem(ofstream& ofstr, const UnivType *pUnivType);

void WriteScalarItem(ofstream& ofstr, const UnivType *pUnivType)
{
	if(!pUnivType->isValid()){
		throw CdbException(CDB_NOT_IMPLEMENTED);
	}else if(pUnivType->isEmpty()){
		throw CdbException(CDB_NOT_IMPLEMENTED);
	}else if(pUnivType->isMatrix()){
		ofstr << '[';
		WriteVectorItem(ofstr, pUnivType);
		ofstr << ']';
	}else{
		assert(pUnivType->isScalar());

		if(pUnivType->isInteger()){
			ofstr << dec;
			ofstr << pUnivType->getInt();
		}else if(pUnivType->isUInteger()){
			ofstr << dec;
			ofstr << pUnivType->getInt();
			ofstr << "UL";
		}else if(pUnivType->isReal()){
			ofstr << showpoint;
			ofstr << pUnivType->getReal();
		}else if(pUnivType->isPointer()){
			ofstr << "0x";
			ofstr << hex;
			ofstr << (unsigned int)pUnivType->getPointer();
		}else if(pUnivType->isBoolean()){
			ofstr << pUnivType->getBool() ? "true" : "false";
		}else if(pUnivType->isString()){
			ofstr << '\"' << pUnivType->getString() << '\"';
		}else if(pUnivType->isBinary()){
			throw CdbException(CDB_NOT_IMPLEMENTED);
		}else if(pUnivType->isSubset()){
			throw CdbException(CDB_NOT_IMPLEMENTED);
		}
	}
}

void WriteVectorItem(ofstream& ofstr, const UnivType *pUnivType)
{
	if(!pUnivType->isValid()){
		throw CdbException(CDB_NOT_IMPLEMENTED);
	}else if(pUnivType->isEmpty()){
		throw CdbException(CDB_NOT_IMPLEMENTED);
	}else if(pUnivType->isMatrix()){
		int row = pUnivType->getNumRow();
		int col = pUnivType->getNumColumn();

		if(row == 1){
			for(int i = 0; i < col; i++){
				if(i != 0){
					ofstr << ", ";
				}
				WriteScalarItem(ofstr, &pUnivType->getAtConst(0, col));
			}
		}else{
			for(int j = 0; j < row; j++){
				if(j != 0){
					ofstr << "; ";
				}
				for(int i = 0; i < col; i++){
					if(i != 0){
						ofstr << ", ";
					}
					WriteScalarItem(ofstr, &pUnivType->getAtConst(row, col));
				}
			}
		}
	}
}

void SerializeEvalValues(const char *pFileName, BaseDB *pDB)
{
	ofstream ofstr(pFileName);
	if(!ofstr){
		throw CdbException(CDB_FILE_OPEN_ERROR, pFileName);
	}

	WrappedMap *pMap = pDB->getContainer();

	const NamedUnivType *pUnivType;
	for(pUnivType = pMap->getNext(true); pUnivType != NULL; pUnivType = pMap->getNext()){

		// ƒ‰ƒxƒ‹–¼
		ofstr << pUnivType->getName();
		if(pUnivType->isScalar()){
			ofstr << " = ";
			WriteScalarItem(ofstr, pUnivType);
			ofstr << endl;
		}else{
			throw CdbException(CDB_NOT_IMPLEMENTED);
		}
	}
}
