#ifndef DEF_COMMON_UTILS
#define DEF_COMMON_UTILS

// ���ʃ��[�e�B���e�B

// NULL��`
#ifndef NULL
#define NULL 0
#endif

// TRUE, FALSE�̒�`
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// �L�[�o�^�̍ő吔
#define CDB_MAX_KEY 500
// �z��v�f�̍ő吔
#define CDB_ARRAY_MAX 512
// �ǂݍ��݃t�@�C���T�C�Y�̍ő�
#define CDB_MAX_FILE_SIZE (180*1024)
// �P�̃g�[�N���̍ő�T�C�Y
#define CDB_MAX_TOKEN_SIZE (80*3)
// 1�s�̍ő�T�C�Y
#define CDB_LINE_MAX_CHAR 512
// �o�^�\�ȃo�C�i���I�u�W�F�N�g�̍ő吔
#define CDB_MAX_BINARY_SIZE (200*1024)

#define CDB_TOKEN_EMPTY 1

#endif
