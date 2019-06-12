#include "TokenAnaly.h"
#include "CdbException.h"
#include "CommonUtils.h"
#include <cassert>
#include <cstring>
#include <cctype>

struct TokenNotation{
	const char *pKey;
	int nLength;
	enum TokenKind eKind;
};

// �y���Ӂz�e�g�[�N���̃v���t�B�b�N�X������ȑO�ɋL�q����Ă���g�[�N����
//  ��v���Ȃ��悤�ɏ��ԂɋC�����邱��
static const struct TokenNotation g_TokenNotations[] = {
	{",",  1, TK_COMMA},
	{":",  1, TK_COLON},
	{";",  1, TK_SEMICOLON},
	{"'",  1, TK_APOS},
	{".'", 2, TK_DOT_APOS},

	{"(",  1, TK_BRACKET_RND_OPEN},
	{")",  1, TK_BRACKET_RND_CLOSE},
	{"[",  1, TK_BRACKET_SQR_OPEN},
	{"]",  1, TK_BRACKET_SQR_CLOSE},
	{"{",  1, TK_BRACE_OPEN},
	{"}",  1, TK_BRACE_CLOSE},
	{"$",  1, TK_SCALAR_EQN},	// �i�X�J���[�]����$$�F�O�o�[�W��������̌݊����̂��߁j

	{"==", 2, TK_CND_EQUAL},
	{"!=", 2, TK_CND_NEQUAL},
	{">=", 2, TK_CND_ELARGE},
	{"<=", 2, TK_CND_ESMALL},
	{">",  1, TK_CND_LARGE},
	{"<",  1, TK_CND_SMALL},

	{"+",  1, TK_PLUS},
	{"-",  1, TK_MINUS},
	{"*",  1, TK_MULT},
	{"/",  1, TK_DIV},
	{".*", 2, TK_DOT_MULT},
	{"./", 2, TK_DOT_DIV},
	{"%",  1, TK_MOD},
	{"&&", 2, TK_LOGICAL_AND},
	{"||", 2, TK_LOGICAL_OR},
	{"!",  1, TK_LOGICAL_NOT},
//	{"^^", 2, TK_LOGICAL_XOR}, 2010/06/20 �p�~
	{"&",  1, TK_BIT_AND},
	{"|",  1, TK_BIT_OR},
	{"~",  1, TK_BIT_NOT},
//	{"^",  1, TK_BIT_XOR}, 2010/06/20 �p�~
	{"^",  1, TK_POWER},
	{".^", 2, TK_DOT_POWER},

	{"=",  1, TK_SUBST},
	{".",  1, TK_DOT},

	{NULL, 0, TK_INVALID}
};

// �\���ꗗ
static const struct TokenNotation g_ReservedStrings[] = {
	{"const",     5, TK_CONST},
	{"true",      4, TK_BOOL_TRUE},
	{"false",     5, TK_BOOL_FALSE},
	{"function",  8, TK_FUNC},
	{"begin",     5, TK_BEGIN},
	{"end",       3, TK_END},
	{"subset",    6, TK_SUBSET},
	{"variable",  8, TK_VARIABLE},
	{"reference", 9, TK_REFERENCE},
	{"if",        2, TK_IF},
	{"then",      4, TK_THEN},
	{"else",      4, TK_ELSE},
	{"elsif",     5, TK_ELSIF},
	{"switch",    6, TK_SWITCH},
	{"case",      4, TK_CASE},
	{"otherwise", 9, TK_OTHERWISE},
	{"where",     5, TK_WHERE},

	{NULL,        0, TK_INVALID}
};

TokenAnaly::TokenAnaly(const char *pSource, size_t sizeSave/* = 0L*/)
{
	assert(pSource != NULL);

	m_nCurrentIndex = 0;

	m_pSource = pSource;

	m_sizeSave = sizeSave;
	if(m_sizeSave != 0L){
		m_pSave = new char[m_sizeSave];
		assert(m_pSave != NULL);
	}else{
		m_pSave = NULL;
	}

	m_pSaveEnd = m_pSave + m_sizeSave;	// �������݉\�Ȗ����n�_

	for(int i = 0; i < TOKEN_NUM_HISTORY; i++){
		m_PastTokenIsDirty[i] = false;
		m_PastTokenLength[i] = 0;
		m_PastTokenSaveLength[i] = 0;
	}

	m_PastLineNumber[0] = 1;	// �s�̃J�E���g�͂P����n�߂�

	m_pPastTokenBegin[0] = pSource;
	m_pPastTokenSaveBegin[0] = m_pSave;
}

TokenAnaly::~TokenAnaly()
{
	delete[] m_pSave;
}

enum TokenKind TokenAnaly::getNext()
{
	analyzeNext();
	return m_PastTokenKind[m_nCurrentIndex % TOKEN_NUM_HISTORY];
}

bool TokenAnaly::putBack(int nBack/* = 1*/)
{
	if(m_nCurrentIndex - nBack >= 0){
		m_nCurrentIndex -= nBack;
		return true;
	}else{
		return false;
	}
}

UnivType& TokenAnaly::getCurrentContent()
{
	return m_PastContents[m_nCurrentIndex % TOKEN_NUM_HISTORY];
}

int TokenAnaly::getCurrentLine()
{
	return m_PastLineNumber[m_nCurrentIndex % TOKEN_NUM_HISTORY];
}

void TokenAnaly::setCurrentContent(const UnivType& x)
{
	int index = m_nCurrentIndex % TOKEN_NUM_HISTORY;
	m_PastTokenIsDirty[index] = false;
	m_PastContents[index].copy(x);
}

void TokenAnaly::analyzeNext()
{
	// �O�g�[�N���̊J�n�ʒu����肷��
	int index = m_nCurrentIndex % TOKEN_NUM_HISTORY;
	const char *pReader = m_pPastTokenBegin[index];
	char *pWriter = m_pPastTokenSaveBegin[index];
	m_nCurrentLineNum = m_PastLineNumber[index];

	// �O�g�[�N���̓��e���R�s�[����
	if(m_pSave != NULL){
		if(m_PastTokenIsDirty[index]){
			const char *pToken = m_Format.ToString(m_PastContents[index]);
			m_PastTokenSaveLength[index] = strlen(pToken);
			strcpy(pWriter, pToken);
		}else{
			strncpy(pWriter, pReader, m_PastTokenSaveLength[index]);
		}
	}

	// ���g�[�N���̊J�n�ʒu����肷��
	pReader += m_PastTokenLength[index];
	pWriter += m_PastTokenSaveLength[index];

	// �󔒂ƃR�����g�̓ǂݔ�΂�
	if(m_pSave == NULL){
		throwComments(pReader);
	}else{
		throwComments(pReader, pWriter);
	}

	// �J�E���^��i�߂āA���̃g�[�N�������Ɉڂ�
	m_nCurrentIndex++;
	index = m_nCurrentIndex % TOKEN_NUM_HISTORY;
	m_PastTokenIsDirty[index] = false;
	m_pPastTokenBegin[index] = pReader;
	m_pPastTokenSaveBegin[index] = pWriter;

	// �t�@�C���̏I���̏ꍇ
	if(*pReader == '\0'){
		m_PastTokenKind[index] = TK_EMPTY;
		m_PastContents[index].clear();
		m_PastTokenLength[index] = 0;
		m_PastLineNumber[index] = m_nCurrentLineNum;
		m_PastTokenSaveLength[index] = 0;
		return;
	}

	// �L���g�[�N����v�̌���
	int i;
	for(i = 0; g_TokenNotations[i].pKey != NULL; i++){
		if(strncmp(g_TokenNotations[i].pKey, pReader, g_TokenNotations[i].nLength) == 0){
			// ��v�����猈��
			m_PastTokenKind[index] = g_TokenNotations[i].eKind;
			m_PastContents[index].clear();	// �L���̏ꍇ�͒��g�Ȃ�
			m_PastTokenLength[index] = g_TokenNotations[i].nLength;
			m_PastLineNumber[index] = m_nCurrentLineNum;
			m_PastTokenSaveLength[index] = g_TokenNotations[i].nLength;
			return;
		}
	}

	// �\���̈�v�̌���
	for(i = 0; g_ReservedStrings[i].pKey != NULL; i++){
		if(strncmp(g_ReservedStrings[i].pKey, pReader, g_ReservedStrings[i].nLength) == 0){
			// ���̕������A���t�@�x�b�g�E���l��������\���ł͂Ȃ�
			const char *pNext = pReader + g_ReservedStrings[i].nLength;
			if(isalnum(*pNext) || *pNext == '_') continue;
			// ��v�����猈��
			m_PastTokenKind[index] = g_ReservedStrings[i].eKind;
			m_PastContents[index].clear();	// �\���̏ꍇ�͒��g�Ȃ�
			m_PastTokenLength[index] = g_ReservedStrings[i].nLength;
			m_PastLineNumber[index] = m_nCurrentLineNum;
			m_PastTokenSaveLength[index] = g_ReservedStrings[i].nLength;
			return;
		}
	}

	// �����񂩂ǂ�������
	if(*pReader == '"'){
		int beginLineNum = m_nCurrentLineNum;
		const char *p;
		for(p = pReader + 1; (*p != '\0') && (*p != '"'); p++){
			// ���s�̃J�E���g
			if((*p == '\r') || (*p == '\n')){
				bool hasNext = (*p == '\r');
				if(hasNext && *(p + 1) == '\n') p++;
				m_nCurrentLineNum++;
			}
		}
		if(*p == '"') p++;
		if(*p == '\0'){
			m_nCurrentLineNum = beginLineNum;
			throw CdbException(CDB_MISSING_DBL_QUOTE);
		}

		int nTokenLength = p - pReader;
		m_PastTokenKind[index] = TK_STRING;
		m_PastContents[index].move(m_Format.ToUnivType(pReader, p));
		m_PastTokenLength[index] = nTokenLength;
		m_PastLineNumber[index] = beginLineNum;
		m_PastTokenSaveLength[index] = nTokenLength;
		return;
	}

	// �o�C�i�����ǂ�������
	if(*pReader == '@'){
		int beginLineNum = m_nCurrentLineNum;
		const char *p;
		for(p = pReader + 1; (*p != '\0') && (*p != '@'); p++){
			// ���s�̃J�E���g
			if((*p == '\r') || (*p == '\n')){
				bool hasNext = (*p == '\r');
				if(hasNext && *(p + 1) == '\n') p++;
				m_nCurrentLineNum++;
			}
		}
		if(*p == '@') p++;
		if(*p == '\0'){
			m_nCurrentLineNum = beginLineNum;
			throw CdbException(CDB_MISSING_AT_MARK);
		}

		int nTokenLength = p - pReader;
		m_PastTokenKind[index] = TK_BINARY;
		m_PastContents[index].move(m_Format.ToUnivType(pReader, p));
		m_PastTokenLength[index] = nTokenLength;
		m_PastLineNumber[index] = beginLineNum;
		m_PastTokenSaveLength[index] = nTokenLength;
		return;
	}

	// �V���{���E���l�𒊏o����
	const char *pTokenEnd = NULL;
	m_PastContents[index].move(m_Format.ToUnivTypeWithoutNewLine(pReader, pTokenEnd));
	if((pReader != pTokenEnd) && (pTokenEnd != NULL)){
		UnivType& x = m_PastContents[index];
		if(x.isInteger()){
			m_PastTokenKind[index] = TK_INT;
		}else if(x.isUInteger()){
			m_PastTokenKind[index] = TK_UINT;
		}else if(x.isReal()){
			m_PastTokenKind[index] = TK_REAL;
		}else if(x.isPointer()){
			m_PastTokenKind[index] = TK_POINTER;
		}else if(x.isString()){
			m_PastTokenKind[index] = TK_SYMBOL;
		}

		int nTokenLength = pTokenEnd - pReader;
		m_pPastTokenBegin[index] = pReader;
		m_pPastTokenSaveBegin[index] = pWriter;
		m_PastTokenLength[index] = nTokenLength;
		m_PastTokenSaveLength[index] = nTokenLength;
		m_PastLineNumber[index] = m_nCurrentLineNum;
		return;
	}

	// �ȏ�A�S�Ăɓ��Ă͂܂�Ȃ������疳���Ƃ���
	m_PastTokenKind[index] = TK_INVALID;
	m_PastTokenLength[index] = 0;
	m_PastTokenSaveLength[index] = 0;
	m_PastLineNumber[index] = m_nCurrentLineNum;
}

void TokenAnaly::throwComments(const char *&pReader)
{
	const char *pBefore;
	do{
		pBefore = pReader;

		// ���s�̏ꍇ
		if((*pReader == '\r') || (*pReader == '\n')){
			bool hasNext = (*pReader == '\r');
			pReader++;
			if(hasNext && *pReader == '\n') pReader++;
			m_nCurrentLineNum++;
			continue;
		}

		// �R�����g�̌��o
		if(*pReader == '#'){
		//if((*pReader == '#') || (*pReader == '%')){	// 20110203 %��mod�̈Ӗ��Ŏg���Ȃ��Ȃ�̂�%�R�����g�͔p�~����
			while((*pReader != '\r') && (*pReader != '\n') && (*pReader != '\0')) pReader++;
			continue;
		}

		// �󔒂ƃ^�u�̓ǂݔ�΂�
		while((*pReader == ' ') || (*pReader == '\t')) pReader++;

	}while((pBefore != pReader) && (*pReader != '\0'));
}

void TokenAnaly::throwComments(const char *&pReader, char *&pWriter)
{
	bool withinComment = false;
	const char *pBefore;
	do{
		// �������ݐ�o�b�t�@���`�F�b�N
		if(pWriter + 2 < m_pSaveEnd) break;

		pBefore = pReader;

		// ���s�̏ꍇ
		if((*pReader == '\r') || (*pReader == '\n')){
			bool hasNext = (*pReader == '\r');
			*pWriter++ = *pReader++;
			if(hasNext && *pReader == '\n') *pWriter++ = *pReader++;
			m_nCurrentLineNum++;
			withinComment = false;
			continue;
		}

		// �R�����g�̓ǂݔ�΂�
		if(withinComment){
			*pWriter++ = *pReader++;
			continue;
		}

		// �R�����g�̌��o
		if((*pReader == '#') || (*pReader == '%')){
			*pWriter++ = *pReader++;
			withinComment = true;
			continue;
		}

		// �󔒂ƃ^�u�̓ǂݔ�΂�
		if((*pReader == ' ') || (*pReader == '\t')){
			*pWriter++ = *pReader++;
			continue;
		}

	}while((pBefore != pReader) && (*pReader != '\0'));
}
