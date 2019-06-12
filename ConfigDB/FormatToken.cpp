#include "FormatToken.h"
#include "CdbException.h"
#include <cstring>
#include <cassert>
#include <cctype>
#include <cstdio>

#define DEFAULT_STRING_BUF_SIZE 1024

FormatToken::FormatToken()
{
	m_sizeStringBuf = DEFAULT_STRING_BUF_SIZE;
	m_pStringBuf = new char[m_sizeStringBuf];
	assert(m_pStringBuf != NULL);
}

FormatToken::~FormatToken()
{
	delete[] m_pStringBuf;
}

void FormatToken::ReallocStringBuf(size_t size)
{
	if(m_sizeStringBuf >= size) return;
	delete[] m_pStringBuf;
	m_sizeStringBuf = size;
	m_pStringBuf = new char[m_sizeStringBuf];
	assert(m_pStringBuf != NULL);
}

const char *FormatToken::ToString(const UnivType& x)
{
	if(x.isString()){
		ReallocStringBuf(x.getStringLength() + 3);
		strcpy(m_pStringBuf, "\"");
		strcat(m_pStringBuf, (const char *)x);
		strcat(m_pStringBuf, "\"");
	}else if(x.isBinary()){
		throw CdbException(CDB_NOT_IMPLEMENTED);	// not implemented yet ...
	}else if(x.isBoolean()){
		strcpy(m_pStringBuf, (bool)x ? "true" : "false");
	}else if(x.isInteger()){
		sprintf(m_pStringBuf, "%dL", (int)x);
	}else if(x.isUInteger()){
		sprintf(m_pStringBuf, "%dU", (unsigned int)x);
	}else if(x.isReal()){
		sprintf(m_pStringBuf, "%gF", (UnivType::Real)x);
	}else if(x.isDouble()){
		sprintf(m_pStringBuf, "%gF", (UnivType::Double)x);
	}else if(x.isPointer()){
		sprintf(m_pStringBuf, "0x%xP", reinterpret_cast<unsigned int>(x.getPointer()));
	}else{
		throw CdbException(CDB_CANNOT_CONV_STR, x.getContentTypeID());
		return NULL;
	}

	return m_pStringBuf;
}

const char *FormatToken::ToStringWithoutSuffix(const UnivType& x)
{
	if(x.isString()){
		ReallocStringBuf(x.getStringLength() + 3);
		strcpy(m_pStringBuf, "\"");
		strcat(m_pStringBuf, (const char *)x);
		strcat(m_pStringBuf, "\"");
	}else if(x.isBinary()){
		assert(false);	// not implemented yet ...
	}else if(x.isBoolean()){
		strcpy(m_pStringBuf, (bool)x ? "true" : "false");
	}else if(x.isInteger()){
		sprintf(m_pStringBuf, "%d", (int)x);
	}else if(x.isUInteger()){
		sprintf(m_pStringBuf, "%d", (unsigned int)x);
	}else if(x.isReal()){
		sprintf(m_pStringBuf, "%g", (UnivType::Real)x);
	}else if(x.isDouble()){
		sprintf(m_pStringBuf, "%g", (UnivType::Double)x);
	}else if(x.isPointer()){
		sprintf(m_pStringBuf, "0x%x", reinterpret_cast<unsigned int>(x.getPointer()));
	}else{
		throw CdbException(CDB_CANNOT_CONV_STR, x.getContentTypeID());
		return NULL;
	}

	return m_pStringBuf;
}

UnivType& FormatToken::ToUnivType(const char *pString)
{
	int nLength = strlen(pString);
	return ToUnivType(pString, pString + nLength);
}

UnivType& FormatToken::ToUnivType(const char *pStringBegin, const char *pStringEnd)
{
	if(*pStringBegin == '"'){
		// 文字列の場合
		int nLength = pStringEnd - pStringBegin - 2;
		ReallocStringBuf(nLength + 1);
		strncpy(m_pStringBuf, pStringBegin + 1, nLength);
		m_pStringBuf[nLength] = '\0';
		assert(*(pStringEnd - 1) == '"');
		m_Data.setString(m_pStringBuf);
	}else if(*pStringBegin == '@'){
		// バイナリオブジェクトの場合
		int nLength = pStringEnd - pStringBegin - 2;
		ReallocStringBuf(nLength + 1);
		strncpy(m_pStringBuf, pStringBegin + 1, nLength);
		m_pStringBuf[nLength] = '\0';
		assert(*(pStringEnd - 1) == '@');
		throw CdbException(CDB_NOT_IMPLEMENTED);	// not implemented yet ...
	}else{
		assert(false);
	}

	return m_Data;
}

UnivType& FormatToken::ToUnivTypeWithoutNewLine(const char *pStringBegin)
{
	const char *pStringEnd;
	return ToUnivTypeWithoutNewLine(pStringBegin, pStringEnd);
}

UnivType& FormatToken::ToUnivTypeWithoutNewLine(const char *pStringBegin, const char *&pStringEnd)
{
	pStringEnd = pStringBegin;
	const char *pElement = pStringBegin;

	// １６進数の場合(大文字)
	if(pElement[0] == '0' && pElement[1] == 'X'){
		int i;
		for(i = 2; isdigit(pElement[i]) || (('A' <= pElement[i]) && (pElement[i] <= 'F')); i++);
		pStringEnd = &pElement[i];

		// 終端をNULLとするために一旦バッファにコピー
		int nLength = i - 2;
		ReallocStringBuf(nLength + 1);
		strncpy(m_pStringBuf, &pElement[2], nLength);
		m_pStringBuf[nLength] = '\0';

		unsigned int val;
		sscanf(m_pStringBuf, "%X", &val);
		m_Data.setUInt(val);

		// ポインタの場合
		if((*pStringEnd == 'p') || (*pStringEnd == 'P')){
			m_Data.setPointer(reinterpret_cast<void *>(val));
			pStringEnd++;
		}
	
	// １６進数の場合(小文字)
	}else if(pElement[0] == '0' && pElement[1] == 'x'){
		int i;
		for(i = 2; isdigit(pElement[i]) || (('a' <= pElement[i]) && (pElement[i] <= 'f')); i++);
		pStringEnd = &pElement[i];

		// 終端をNULLとするために一旦バッファにコピー
		int nLength = i - 2;
		ReallocStringBuf(nLength + 1);
		strncpy(m_pStringBuf, &pElement[2], nLength);
		m_pStringBuf[nLength] = '\0';

		unsigned int val;
		sscanf(m_pStringBuf, "%x", &val);
		m_Data.setUInt(val);
	
		// ポインタの場合
		if((*pStringEnd == 'p') || (*pStringEnd == 'P')){
			m_Data.setPointer(reinterpret_cast<void *>(val));
			pStringEnd++;
		}

	// ２進数の場合
	}else if(pElement[0] == '0' && (pElement[1] == 'b' || pElement[1] == 'B')){
		const char *p = &pElement[2];
		const char *pEnd = p;
		while(*pEnd == '0' || *pEnd == '1') pEnd++;
		pStringEnd = pEnd;
		
		unsigned int val = 0;
		int i;
		for(i = -1; (p - 1) != pEnd; i++, pEnd--){
			if(i < 0) continue;
			if(*pEnd == '1') val = val + ((unsigned int)1 << i);
		}
		m_Data.setUInt(val);

	// １０進数の場合
	}else if(isdigit(pElement[0])){
		const char *pDigit = pElement;

		bool isFoundDot = false;
		while(isdigit(*pDigit)) pDigit++;
		if(*pDigit == '.'){
			pDigit++;
			isFoundDot = true;
		}
		while(isdigit(*pDigit)) pDigit++;

		char suffix1 = *pDigit;
		char suffix2 = (suffix1 != '\0') ? *(pDigit + 1) : '\0';

		bool isInt = false, isUInt = false, isReal = false, isPointer = false;
		switch(suffix1){
		case 'u':
		case 'U':
			isUInt = true;
			if((suffix2 == 'l') || (suffix2 == 'L')){
				pStringEnd = pDigit + 2;
			}else{
				pStringEnd = pDigit + 1;
			}
			if(isFoundDot) throw CdbException(CDB_ILLEGAL_FORMAT, "Unsigned integer must not have point");
			break;
		case 'l':
		case 'L':
			isInt = true;
			pStringEnd = pDigit + 1;
			if(isFoundDot) throw CdbException(CDB_ILLEGAL_FORMAT, "Integer must not have point");
			break;
		case 'f':
		case 'F':
			isReal = true;
			pStringEnd = pDigit + 1;
			break;
		case 'p':
		case 'P':
			isPointer = true;
			pStringEnd = pDigit + 1;
			if(isFoundDot) throw CdbException(CDB_ILLEGAL_FORMAT, "Pointer must be non-negative integer");
			break;
		default:
			if(isFoundDot){
				isReal = true;
			}else{
				isInt = true;
			}
			pStringEnd = pDigit;
		}

		// 終端をNULLとするために一旦バッファにコピー
		int nLength = pDigit - pElement;
		ReallocStringBuf(nLength + 1);
		strncpy(m_pStringBuf, pElement, nLength);
		m_pStringBuf[nLength] = '\0';

		// 実数
		if(isReal){
			float val;
			sscanf(m_pStringBuf, "%f", &val);
			m_Data.setReal(val);
		
		// 整数
		}else if(isInt){
			int val;
			sscanf(m_pStringBuf, "%d", &val);
			m_Data.setInt(val);
		
		// 符号なし整数
		}else if(isUInt){
			unsigned int val;
			sscanf(m_pStringBuf, "%u", &val);
			m_Data.setUInt(val);

		// ポインタ
		}else if(isPointer){
			unsigned int val;
			sscanf(m_pStringBuf, "%u", &val);
			m_Data.setPointer(reinterpret_cast<void *>(val));
		}


	// 文字列の場合
	}else{
		int i;
		for(i = 0; isalnum(pElement[i]) || (pElement[i] == '_'); i++);

		// 終端をNULLとするために一旦バッファにコピー
		int nLength = i;
		ReallocStringBuf(nLength + 1);
		strncpy(m_pStringBuf, pElement, nLength);
		m_pStringBuf[nLength] = '\0';

		m_Data.setString(m_pStringBuf);
		pStringEnd = &pElement[i];

	}

	return m_Data;
}
