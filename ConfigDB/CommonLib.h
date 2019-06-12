/*********************************************************************
	Rhapsody	: 3.0.1 
	Login		: ff99045
	Component	: TestPc 
	Configuration 	: DefaultConfig
	Model Element	: CommonLibPc
//!	Generated Date	: Tue, 3, Apr 2007  
	File Path	: TestPc\DefaultConfig\CommonLibPc.h
*********************************************************************/

#ifndef CommonLibPc_H 
#define CommonLibPc_H 

#include <cstdio>

#define CLIB_USE_CPP_CLI

//----------------------------------------------------------------------------
// CommonLibPc.h                                                                  
//----------------------------------------------------------------------------

// ���ʃ��C�u���� - �o�b�p

#define CLIB_OK 0
#define CLIB_ERROR (-1)
#define CLIB_ERROR_FOPEN (-2)
#define CLIB_WRONG_SIZE (-3)
#define CLIB_MAX_FILENAME 128

// �f�t�H���g�̃t�@�C���p�X��ݒ肷��
void clib_set_default_path(const char *pFileName);

// �f�t�H���g�̃t�@�C���p�X�𓾂�
extern "C" const char * clib_get_default_path();

// �f�B���N�g���w��̃t���p�X��
extern "C" void clib_get_full_path(char *pFileName, int nsize);

// �t�@�C���ǂݍ���
extern "C" int clib_file_read(const char*  pFileName, void *  pBuff, int  size);

// �t�@�C����������
extern "C" int clib_file_write(const char*  pFileName, const void*  pBuff, int  size);

// �t�@�C���T�C�Y�̎擾
extern "C" int clib_get_file_size(const char *pFileName);

// TDT�t�@�C���`���ł̉摜�t�@�C�������o���B
// �y���ӁzpFileName�ɂ͊g���q���܂߂Ȃ����ƁB
int clib_write_tdt(const char* pFileName, const unsigned short* pBuff, int  nMain, int  nSub);

// TMA�w�b�_�̃t�@�C�������o���B
// �y���ӁzpFileName�ɂ͊g���q���܂߂Ȃ����ƁB
int clib_write_tma(const char* pFileName, int  nMain, int  nSub);

// TDT�t�@�C����ǂݍ��ށB
// �y���ӁzpFileName�ɂ͊g���q���܂߂Ȃ����ƁB
int clib_read_tdt(const char* pFileName, unsigned short* pBuff, int nMain, int nSub);

// TMA�w�b�_��ǂݍ���ŁA�摜�̃T�C�Y�𓾂�B
// �y���ӁzpFileName�ɂ͊g���q���܂߂Ȃ����ƁB
int clib_read_tma(const char* pFileName, int& nMain, int& nSub);

// �L���b�V���֎~�̗̈���m�ۂ���
extern "C" void * clib_allocate_nocache(int size);

// �L���b�V���֎~�̗̈���J������
extern "C" bool clib_free_nocache(void *p);

#ifndef CLIB_USE_CPP_CLI
// Unicode��Ansi�ϊ�
char* UnicodeToAnsi(const _TCHAR *pUnicode);
#endif

#endif

/*********************************************************************
	File Path	: TestPc\DefaultConfig\CommonLibPc.h
*********************************************************************/

