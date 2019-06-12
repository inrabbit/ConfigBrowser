#include "CdbException.h"
#include "UnivType.h"
#include <cassert>
#include <cstdio>
#include <cstdarg>

// エラーメッセージの最大長
#define CDB_MAX_ERROR_MSG_LEN 256

using namespace std;

struct CdbException::ExceptionMap CdbException::m_ExceptionMap[] = {
	{CDB_NOTFOUND               , 1, "Variable %s is not found"},
	{CDB_KEY_OVERFLOW           , 1, ""},
	{CDB_KEY_COLLISION          , 1, "Variable %s is already defined"},
	{CDB_ARRAY_OVERFLOW         , 1, ""},
	{CDB_FILE_OPEN_ERROR        , 1, "Cannot open the file %s"},
	{CDB_ILLEGAL_FORMAT         , 1, "Illegal format : %s"},
	{CDB_FILE_REMOVE_ERROR      , 1, "Cannot remove the file %s"},
	{CDB_FILE_RENAME_ERROR      , 1, "Cannot rename the file %s"},
	{CDB_RANGE_OVER             , 1, ""},
	{CDB_BAD_ALLOC              , 0, "Cannot allocate memory"},
	{CDB_ILLEGAL_TYPE           , 1, ""},
	{CDB_BINARY_SIZEOVER        , 1, ""},
	{CDB_EQN_ERROR              , 0, ""},
	{CDB_CONNECT_ERROR          , 0, "Database connect error"},
	{CDB_ALRDY_CONNECTED        , 0, "Database already connected"},
	{CDB_CANNOT_DISCONNECT      , 1, "Cannot disconnect the database"},
	{CDB_WRITE_TO_CONST         , 0, "Write to constant variable"},
	{CDB_WRITE_TO_CONST1        , 1, "Write to constant variable : %s"},
	{CDB_MISSING_DBL_QUOTE      , 0, "Missing double quotation"},
	{CDB_MISSING_AT_MARK        , 0, "Missing at-mark"},
	{CDB_CANNOT_CONV_STR        , 1, "Cannot convert to string : current type-ID is %d"},
	{CDB_CONFLICT_FUNC          , 1, "Function name conflict : %s is already defined"},
	{CDB_DEFINITION_NOT_UNIQUE  , 0, "Definition is not unique"},
	{CDB_DEFINITION_NOT_UNIQUE1 , 1, "Definition is not unique : %s is multiply-defined"},
	{CDB_INDEX_EXCEED           , 0, "Array index exceed"},
	{CDB_INDEX_EXCEED1          , 1, "Array index exceed : index = %d"},
	{CDB_INDEX_EXCEED2          , 2, "Array index exceed : row = %d, col = %d"},
	{CDB_INDEX_NEGATIVE         , 0, "Negative array index"},
	{CDB_INDEX_NEGATIVE1        , 1, "Negative array index : index = %d"},
	{CDB_INDEX_NEGATIVE2        , 2, "Negative array index : row = %d, col = %d"},
	{CDB_CFUNC_INVALID_NARG     , 1, "Incorrect number of arguments to custom function %s"},
	{CDB_CFUNC_INVALID_NARG2    , 2, "Incorrect number of arguments to custom function : %d is set, but %d is expected"},
	{CDB_CFUNC_INVALID_ARG      , 1, "Invalid arguments to custom function %s"},
	{CDB_CFUNC_INVALID_ARG2     , 2, "Invalid arguments to custom function %s : %s"},
	{CDB_CFUNC_INVALID_ARG3     , 3, "The %d-%s argument to the function call of '%s' is invalid"},
	{CDB_SYNTAX_ERROR           , 0, "Syntax error"},
	{CDB_CAST_ERROR             , 0, "Variable cast error"},
	{CDB_NOT_SCALAR             , 0, "Variable is not scalar"},
	{CDB_SERIES_INVALID_PARAM   , 3, "Invalid parameters of series \"%d:%d:%d\""},
	{CDB_EXCEED_MAX_TOKEN_BACK  , 1, "Exceeded max token back : %d"},
	{CDB_CANNOT_COMPARE         , 1, "Cannot compare the element type - %s"},
	{CDB_VAGUE_PRIORITY         , 1, "The order of priority of operation is vague : usage of %s"},
	{CDB_CAT_DIFFERENT_DIM      , 2, "Cannot CAT with respect to %s between different dimensions for %s"},
	{CDB_SELF_REFERENCE         , 0, "Self-reference occured"},
	{CDB_NULL_REFERENCE         , 0, "Reference to NULL pointer"},
	{CDB_NOT_FUNCTION           , 0, "Function call error"},
	{CDB_NOT_FUNCTION1          , 1, "'%s' is not function"},
	{CDB_NOT_ARRAY              , 0, "Array access error"},
	{CDB_NOT_SUBSET             , 0, "Subset access error"},
	{CDB_MATRIX_NOT_CONFORM2    , 2, "Matrix multiplication for not conforming size (col = %d against raw = %d)"},
	{CDB_MATRIX_NOT_CONFORM3    , 3, "Matrix operation for not conforming size (%s : %d against %d)"},
	{CDB_CANNOT_REFERENCE       , 0, "The 'reference' keyword cannot be used here"},
	{CDB_NOT_REFERENCE          , 0, "Reference error"},
	{CDB_MISSING_LINE_END_MARK  , 0, "Missing semicolon at the end of the line"},
	{CDB_REFERENCE_TO_AROP      , 0, "Reference to arithmetic operation"},
	{CDB_REFERENCE_TO_FUNC      , 0, "Reference to function"},
	{CDB_UNKNOWN_TYPE           , 0, "Unknown type"},
	{CDB_COND_REF_OVER          , 0, "reference index overflow in cond(%d)"},
	{CDB_FIXED_FUNC_ARG         , 0, "A number of arguments is fixed"},
	{CDB_NAME_LENGTH_TOOLONG    , 2, "Too long symbol string (length=%d, max=%d)"},
	{CDB_SPRINTF_NOTSUPPORT     , 1, "Not supported format in sprintf(%s)"},

	{CDB_LOGIC_ERROR            , 0, "Logical error in programming"},
	{CDB_NOT_IMPLEMENTED        , 0, "The function is not implemented yet"},
	{CDB_OK                     , 0, NULL}
};


CdbException::CdbException(enum CdbErrorCode ecode, ...) : runtime_error("")
{
	va_list arg;
	va_start(arg, ecode);

	const char *pMessageFormat = NULL;
	for(int i = 0; m_ExceptionMap[i].m_pMessageFormat != NULL; i++){
		if(m_ExceptionMap[i].m_eErrorCode == ecode){
			pMessageFormat = m_ExceptionMap[i].m_pMessageFormat;
			break;
		}
	}
	if(pMessageFormat != NULL){
		char szMessage[CDB_MAX_ERROR_MSG_LEN];
		vsprintf(szMessage, pMessageFormat, arg);
		m_Message = szMessage;
	}

	m_eErrorCode = ecode;

	va_end(arg);
}

void CdbException::addVariableName(const char *pName)
{
	char szMessage[CDB_MAX_ERROR_MSG_LEN];
	sprintf(szMessage, " -> '%s'", pName);
	m_Message += szMessage;
}

void CdbException::addVariableName(const char *pName, int nIndex)
{
	char szMessage[CDB_MAX_ERROR_MSG_LEN];
	sprintf(szMessage, " -> '%s(%d)'", pName, nIndex);
	m_Message += szMessage;
}

void CdbException::addVariableName(const char *pName, int nRowIndex, int nColIndex)
{
	char szMessage[CDB_MAX_ERROR_MSG_LEN];
	sprintf(szMessage, " -> '%s(%d, %d)'", pName, nRowIndex, nColIndex);
	m_Message += szMessage;
}

void CdbException::addFileNameLine(const char *pName, int nLine)
{
	char szMessage[CDB_MAX_ERROR_MSG_LEN];
	sprintf(szMessage, " at \"%s\", line = %d", pName, nLine);
	m_Message += szMessage;
}

void CdbException::addFileName(const char *pName)
{
	char szMessage[CDB_MAX_ERROR_MSG_LEN];
	sprintf(szMessage, " at \"%s\"", pName);
	m_Message += szMessage;
}

void CdbException::addFileLine(int nLine)
{
	char szMessage[CDB_MAX_ERROR_MSG_LEN];
	sprintf(szMessage, " at line = %d", nLine);
	m_Message += szMessage;
}

// ----------------------------------------

const char *getOrdinalNumberSuffix(int n)
{
	switch(n){
	case 1:
		return "st";
	case 2:
		return "nd";
	default:
		return "th";
	}
}
