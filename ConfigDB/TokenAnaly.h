#ifndef DEF_TOKEN_ANALY
#define DEF_TOKEN_ANALY

#define TOKEN_NUM_HISTORY 256	// �ߋ��̉�͌��ʕێ���
#define TOKEN_MAX_BACK	(TOKEN_NUM_HISTORY - 1)	// �ߋ��ő剽�g�[�N���܂ők��邩

#include "UnivType.h"
#include "FormatToken.h"

enum TokenKind{
	TK_COMMA              = 0,
	TK_COLON              = 1,
	TK_SEMICOLON          = 2,
	TK_APOS               = 3,
	TK_DOT_APOS           = 4,
	TK_SUBST              = 5,
	TK_DOT                = 6,

	TK_BRACKET_RND_OPEN   = 10,
	TK_BRACKET_RND_CLOSE  = 11,
	TK_BRACKET_SQR_OPEN   = 12,
	TK_BRACKET_SQR_CLOSE  = 13,
	TK_BRACE_OPEN         = 14,
	TK_BRACE_CLOSE        = 15,
	TK_SCALAR_EQN         = 16,

	// ��r���Z�q
	TK_CND_EQUAL          = 21,
	TK_CND_NEQUAL         = 22,
	TK_CND_LARGE          = 23,
	TK_CND_SMALL          = 24,
	TK_CND_ELARGE         = 25,
	TK_CND_ESMALL         = 26,

	// ���Z�q
	TK_PLUS               = 31,
	TK_MINUS              = 32,
	TK_MULT               = 33,
	TK_DIV                = 34,
	TK_DOT_MULT           = 35,
	TK_DOT_DIV            = 36,
	TK_MOD                = 37,
	TK_BIT_AND            = 38,
	TK_BIT_OR             = 39,
	TK_BIT_NOT            = 40,
//	TK_BIT_XOR            = 41,
	TK_LOGICAL_AND        = 42,
	TK_LOGICAL_OR         = 43,
	TK_LOGICAL_NOT        = 44,
//	TK_LOGICAL_XOR        = 45,
	TK_POWER              = 46,
	TK_DOT_POWER          = 47,

	// �\���
	TK_CONST              = 60,
	TK_BOOL_TRUE          = 61,
	TK_BOOL_FALSE         = 62,
	TK_FUNC               = 63,
	TK_BEGIN              = 64,
	TK_END                = 65,
	TK_SUBSET             = 66,
	TK_VARIABLE           = 67,
	TK_REFERENCE          = 68,
	TK_IF                 = 69,
	TK_THEN               = 70,
	TK_ELSE               = 71,
	TK_ELSIF              = 72,
	TK_SWITCH             = 73,
	TK_CASE               = 74,
	TK_OTHERWISE          = 75,
	TK_WHERE              = 76,

	// �f�[�^�^
	TK_SYMBOL             = 80,	// �V���{���i������j
	TK_STRING             = 81,	// ������i""�ň͂܂ꂽ���́j
	TK_INT                = 82,	// �����i�����_�Ȃ��̏ꍇ�̃f�t�H���g�A������L�j
	TK_UINT               = 83,	// �����Ȃ������i������UL�j
	TK_REAL               = 84,	// �����i�����_���A�������͖�����f�j
	TK_POINTER            = 85,	// �|�C���^�i������p�j
	TK_BOOL               = 86,	// �u�[���itrue ���� false�j
	TK_BINARY             = 87,	// �o�C�i���i@@�ň͂܂ꂽ���́j

	TK_INVALID            = 90,
	TK_EMPTY              = 99,	// ��ł��邱�Ƃ�����
};

class TokenAnaly
{
private:
	// �g�[�N���̃t�H�[�}�b�g��͊�
	FormatToken m_Format;

	// �ߋ��g�[�N���̃Z�[�u
	enum TokenKind m_PastTokenKind[TOKEN_NUM_HISTORY];
	UnivType m_PastContents[TOKEN_NUM_HISTORY];
	const char *m_pPastTokenBegin[TOKEN_NUM_HISTORY];
	size_t m_PastTokenLength[TOKEN_NUM_HISTORY];
	int m_PastLineNumber[TOKEN_NUM_HISTORY];
	char *m_pPastTokenSaveBegin[TOKEN_NUM_HISTORY];
	size_t m_PastTokenSaveLength[TOKEN_NUM_HISTORY];
	bool m_PastTokenIsDirty[TOKEN_NUM_HISTORY];

	// ���݈ʒu
	int m_nCurrentIndex;
	int m_nCurrentLineNum;

	// �\�[�X����
	const char *m_pSource;

	// �Z�[�u���͗̈�
	char *m_pSave;
	char *m_pSaveEnd;
	size_t m_sizeSave;

private:
	// ���g�[�N���̉��
	void analyzeNext();

	// �R�����g�ƃX�y�[�X�̓ǂݔ�΂�
	void throwComments(const char *&pReader);
	void throwComments(const char *&pReader, char *&pWriter);

public:
	TokenAnaly(const char *pSource, size_t sizeSave = 0L);
	~TokenAnaly();

	// �g�[�N����͋@�\�Ɋւ������
	enum TokenKind getNext();
	bool putBack(int nBack = 1);
	UnivType& getCurrentContent();
	int getCurrentLine();
	int getCurrentTokenPos(){ return m_nCurrentIndex; }

	// �Z�[�u�@�\�Ɋւ������
	void setCurrentContent(const UnivType& x);

};

#endif
