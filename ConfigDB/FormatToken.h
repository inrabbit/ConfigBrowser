#ifndef DEF_FORMAT_TOKEN
#define DEF_FORMAT_TOKEN

#include "UnivType.h"

class FormatToken
{
private:
	UnivType m_Data;
	char *m_pStringBuf;
	size_t m_sizeStringBuf;

	void ReallocStringBuf(size_t size);

public:
	FormatToken();
	~FormatToken();

	// *********************************************
	// �f�[�^���當����ɕϊ�
	// *********************************************
	// �^���T�t�B�b�N�X�t
	const char *ToString(const UnivType& x);
	// �^���T�t�B�b�N�X�Ȃ�
	const char *ToStringWithoutSuffix(const UnivType& x);

	// *********************************************
	// �����񂩂�f�[�^�ɕϊ�
	// *********************************************
	// ������^�ƃo�C�i���^�E�g�[�N�����ɉ��s��������́i�I�[NULL�Ɍ���j
	UnivType& ToUnivType(const char *pString);
	// ������^�ƃo�C�i���^�E�g�[�N�����ɉ��s��������́i�I�[�ʒu���|�C���^�Ŏw��j
	UnivType& ToUnivType(const char *pStringBegin, const char *pStringEnd);
	// ������^�ƃo�C�i���^�ȊO�E�g�[�N�����ɉ��s������Ȃ����́i�I�[�ʒu�����o���ĕԂ��j
	UnivType& ToUnivTypeWithoutNewLine(const char *pStringBegin, const char *&pStringEnd);
	// ������^�ƃo�C�i���^�ȊO�E�g�[�N�����ɉ��s������Ȃ����́i�I�[�ʒu��Ԃ��Ȃ��j
	UnivType& ToUnivTypeWithoutNewLine(const char *pStringBegin);
};

#endif
