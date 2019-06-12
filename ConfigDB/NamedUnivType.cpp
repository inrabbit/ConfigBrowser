#include "NamedUnivType.h"
#include "CdbException.h"
#include <cassert>
#include <cstring>

NamedUnivType::NamedUnivType() : UnivType()
{
	m_type.contentKind = TYPE_INVALID;
	m_type.hasName = FALSE;
	m_pName = NULL;
}

NamedUnivType::~NamedUnivType()
{
	delete[] m_pName;
	UnivType::~UnivType();
}

void NamedUnivType::setName(const char *pName)
{
	if(m_pName != NULL){
		delete[] m_pName;
		m_pName = NULL;
	}

	if(pName != NULL){
		int n = strlen(pName);
		if(n >= MAX_NAME_LENGTH){
			throw CdbException(CDB_NAME_LENGTH_TOOLONG, n, MAX_NAME_LENGTH);
		}
		m_pName = new char[n + 1];
		strcpy(m_pName, pName);

		// ���O����ɐݒ�
		m_type.hasName = TRUE;
	}else{

		// ���O�Ȃ��ɐݒ�
		m_type.hasName = FALSE;
	}

	// ���O���I�u�W�F�N�g�̓f�t�H���g�Œ萔�Ƃ͂Ȃ�Ȃ�
	m_type.isConstant = FALSE;
}
