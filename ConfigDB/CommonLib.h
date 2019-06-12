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

// 共通ライブラリ - ＰＣ用

#define CLIB_OK 0
#define CLIB_ERROR (-1)
#define CLIB_ERROR_FOPEN (-2)
#define CLIB_WRONG_SIZE (-3)
#define CLIB_MAX_FILENAME 128

// デフォルトのファイルパスを設定する
void clib_set_default_path(const char *pFileName);

// デフォルトのファイルパスを得る
extern "C" const char * clib_get_default_path();

// ディレクトリ指定のフルパス化
extern "C" void clib_get_full_path(char *pFileName, int nsize);

// ファイル読み込み
extern "C" int clib_file_read(const char*  pFileName, void *  pBuff, int  size);

// ファイル書き込み
extern "C" int clib_file_write(const char*  pFileName, const void*  pBuff, int  size);

// ファイルサイズの取得
extern "C" int clib_get_file_size(const char *pFileName);

// TDTファイル形式での画像ファイル書き出し。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_write_tdt(const char* pFileName, const unsigned short* pBuff, int  nMain, int  nSub);

// TMAヘッダのファイル書き出し。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_write_tma(const char* pFileName, int  nMain, int  nSub);

// TDTファイルを読み込む。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_read_tdt(const char* pFileName, unsigned short* pBuff, int nMain, int nSub);

// TMAヘッダを読み込んで、画像のサイズを得る。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_read_tma(const char* pFileName, int& nMain, int& nSub);

// キャッシュ禁止の領域を確保する
extern "C" void * clib_allocate_nocache(int size);

// キャッシュ禁止の領域を開放する
extern "C" bool clib_free_nocache(void *p);

#ifndef CLIB_USE_CPP_CLI
// Unicode→Ansi変換
char* UnicodeToAnsi(const _TCHAR *pUnicode);
#endif

#endif

/*********************************************************************
	File Path	: TestPc\DefaultConfig\CommonLibPc.h
*********************************************************************/

