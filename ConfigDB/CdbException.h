#ifndef DEF_CDB_EXCEPTION
#define DEF_CDB_EXCEPTION

#include <stdexcept>
#include <string>
class UnivType;

// エラーコード
typedef enum CdbErrorCode{
	CDB_NOTFOUND               = (-1),
	CDB_KEY_OVERFLOW           = (-2),
	CDB_KEY_COLLISION          = (-3),
	CDB_ARRAY_OVERFLOW         = (-4),
	CDB_FILE_OPEN_ERROR        = (-5),
	CDB_ILLEGAL_FORMAT         = (-6),
	CDB_FILE_REMOVE_ERROR      = (-7),
	CDB_FILE_RENAME_ERROR      = (-8),
	CDB_RANGE_OVER             = (-9),
	CDB_BAD_ALLOC              = (-10),
	CDB_ILLEGAL_TYPE           = (-11),
	CDB_BINARY_SIZEOVER        = (-12),
	CDB_EQN_ERROR              = (-13),
	CDB_CONNECT_ERROR          = (-14),
	CDB_ALRDY_CONNECTED        = (-15),
	CDB_CANNOT_DISCONNECT      = (-16),
	CDB_WRITE_TO_CONST         = (-17),
	CDB_WRITE_TO_CONST1        = (-18),
	CDB_MISSING_DBL_QUOTE      = (-19),
	CDB_MISSING_AT_MARK        = (-40),
	CDB_CANNOT_CONV_STR        = (-41),
	CDB_CONFLICT_FUNC          = (-42),
	CDB_DEFINITION_NOT_UNIQUE  = (-43),
	CDB_DEFINITION_NOT_UNIQUE1 = (-44),
	CDB_INDEX_EXCEED           = (-45),
	CDB_INDEX_EXCEED1          = (-46),
	CDB_INDEX_EXCEED2          = (-47),
	CDB_INDEX_NEGATIVE         = (-48),
	CDB_INDEX_NEGATIVE1        = (-49),
	CDB_INDEX_NEGATIVE2        = (-60),
	CDB_CFUNC_INVALID_NARG     = (-61),
	CDB_CFUNC_INVALID_NARG2    = (-62),
	CDB_CFUNC_INVALID_ARG      = (-63),
	CDB_CFUNC_INVALID_ARG2     = (-64),
	CDB_CFUNC_INVALID_ARG3     = (-65),
	CDB_SYNTAX_ERROR           = (-66),
	CDB_CAST_ERROR             = (-67),
	CDB_NOT_SCALAR             = (-68),
	CDB_SERIES_INVALID_PARAM   = (-69),
	CDB_EXCEED_MAX_TOKEN_BACK  = (-80),
	CDB_CANNOT_COMPARE         = (-81),
	CDB_VAGUE_PRIORITY         = (-82),
	CDB_CAT_DIFFERENT_DIM      = (-83),
	CDB_SELF_REFERENCE         = (-84),
	CDB_NULL_REFERENCE         = (-85),
	CDB_NOT_FUNCTION           = (-86),
	CDB_NOT_FUNCTION1          = (-87),
	CDB_NOT_ARRAY              = (-88),
	CDB_NOT_SUBSET             = (-89),
	CDB_MATRIX_NOT_CONFORM     = (-100),
	CDB_MATRIX_NOT_CONFORM2    = (-101),
	CDB_MATRIX_NOT_CONFORM3    = (-102),
	CDB_CANNOT_REFERENCE       = (-103),
	CDB_NOT_REFERENCE          = (-104),
	CDB_MISSING_LINE_END_MARK  = (-105),
	CDB_REFERENCE_TO_AROP      = (-106),
	CDB_REFERENCE_TO_FUNC      = (-107),
	CDB_UNKNOWN_TYPE           = (-108),
	CDB_COND_REF_OVER          = (-109),
	CDB_FIXED_FUNC_ARG         = (-110),
	CDB_NAME_LENGTH_TOOLONG    = (-111),
	CDB_SPRINTF_NOTSUPPORT     = (-112),

	CDB_LOGIC_ERROR            = (-998),
	CDB_NOT_IMPLEMENTED        = (-999),
	CDB_OK                     = 0
} CDB_ERR;


class CdbException : public std::runtime_error
{
private:
	CDB_ERR m_eErrorCode;
	std::string m_Message;

	struct ExceptionMap {
		CDB_ERR m_eErrorCode;
		int m_nArguments;
		const char *m_pMessageFormat;
	};

	static struct ExceptionMap m_ExceptionMap[];

public:
	CdbException(enum CdbErrorCode ecode, ...);
	virtual const char *what() const{ return m_Message.c_str(); }

public:
	void addVariableName(const char *pName);
	void addVariableName(const char *pName, int nIndex);
	void addVariableName(const char *pName, int nRowIndex, int nColIndex);
	void addFileNameLine(const char *pName, int nLine);
	void addFileName(const char *pName);
	void addFileLine(int nLine);

	CDB_ERR getErrorCode(){ return m_eErrorCode; }
};

const char *getOrdinalNumberSuffix(int n);

#endif
