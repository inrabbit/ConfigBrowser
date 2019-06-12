/********************************************************************
	Rhapsody	: 3.0.1 
	Login		: ff99045
	Component	: TestPc 
	Configuration 	: DefaultConfig
	Model Element	: CommonLibPc
//!	Generated Date	: Wed, 9, May 2007  
	File Path	: TestPc\DefaultConfig\CommonLibPc.cpp
*********************************************************************/

#include "stdafx.h"
#include "CommonLib.h"
#ifdef CLIB_USE_CPP_CLI
#include "../CommonDotNet.h"
#endif
#include <errno.h>
#include <windows.h>
//#include <winbase.h>
#include <iostream>

#undef GetCurrentDirectory	// Win32の方を見ないようにする

using namespace std;

//## package CommonLibPc 

static char g_szCurrentDir[CLIB_MAX_FILENAME] = "";

//----------------------------------------------------------------------------
// CommonLibPc.cpp                                                                  
//----------------------------------------------------------------------------

// デフォルトのファイルパスを設定する
void clib_set_default_path(const char *pFileName)
{
	strncpy(g_szCurrentDir, pFileName, CLIB_MAX_FILENAME);

	// ファイルが指定されているか、フォルダか指定されているかを判定する
	FILE *fp = fopen(g_szCurrentDir,"rb"); 
	if(fp != NULL){
		fclose(fp);
		// 最後にファイル名が記述されているのを消す
		if(strchr(g_szCurrentDir, '\\') != NULL){
			char *p = g_szCurrentDir + strlen(g_szCurrentDir) - 1;
			for(; (p != g_szCurrentDir) && (*p != '\\'); p--);
			if(p == g_szCurrentDir) return;
			*p = NULL;
			strcat(g_szCurrentDir, "\\");
		}
	}
}

// デフォルトのファイルパスを得る
const char * clib_get_default_path()
{
	if(g_szCurrentDir[0] == '\0'){
#ifdef CLIB_USE_CPP_CLI
		System::String^ FileName = System::IO::Directory::GetCurrentDirectory();
		std::string StdFileName = from_cli(FileName);
		strncpy(g_szCurrentDir, StdFileName.c_str(), CLIB_MAX_FILENAME);
		strcat(g_szCurrentDir, "\\");
#else
		::GetCurrentDirectoryA(CLIB_MAX_FILENAME, g_szCurrentDir);
		strcat(g_szCurrentDir, "\\");
#endif
	}

	return g_szCurrentDir;
}

// ディレクトリ指定のフルパス化
void clib_get_full_path(char *pFileName, int nsize)
{
	// 空なら空文字列を返す
	if(pFileName[0] == '\0') return;

	if(strstr(pFileName, ":\\") == NULL && strstr(pFileName, "\\\\") == NULL){
		char szPath[CLIB_MAX_FILENAME];
		strncpy(szPath, clib_get_default_path(), CLIB_MAX_FILENAME);
		strncat(szPath, pFileName, CLIB_MAX_FILENAME);
		strncpy(pFileName, szPath, nsize);
	}
}

// ファイル読み込み
int clib_file_read(const char*  pFileName, void *  pBuff, int  size)
{
	char szPath[CLIB_MAX_FILENAME];
	strncpy(szPath, pFileName, CLIB_MAX_FILENAME);
	clib_get_full_path(szPath, CLIB_MAX_FILENAME);

    FILE *fp;
    int read_size;

    if((fp = fopen(szPath, "rb")) == NULL){
		cerr << strerror(errno) << std::endl;
    	return CLIB_ERROR_FOPEN;
    }
    
    read_size = fread(pBuff, 1, size, fp);
	if(read_size == 0){
		cerr << strerror(errno) << std::endl;
    	return CLIB_ERROR_FOPEN;
    }

    fclose(fp);
    
    return (read_size < 0) ? CLIB_ERROR : read_size;    
}

// ファイル書き込み
int clib_file_write(const char*  pFileName, const void*  pBuff, int  size)
{
	char szPath[CLIB_MAX_FILENAME];
	strncpy(szPath, pFileName, CLIB_MAX_FILENAME);
	clib_get_full_path(szPath, CLIB_MAX_FILENAME);
	
    FILE *fp;
    int write_size;

    if((fp = fopen(szPath, "wb")) == NULL){
		cerr << strerror(errno) << std::endl;
    	return CLIB_ERROR_FOPEN;
    }
    
    write_size = fwrite(pBuff, 1, size, fp);
	if(write_size == 0){
		cerr << strerror(errno) << std::endl;
    	return CLIB_ERROR_FOPEN;
    }

    fclose(fp);
    
    return (write_size < 0) ? CLIB_ERROR : write_size;    
}

int clib_get_file_size(const char *pFileName)
{
	fpos_t fsize = 0;

	char szPath[CLIB_MAX_FILENAME];
	strncpy(szPath, pFileName, CLIB_MAX_FILENAME);
	clib_get_full_path(szPath, CLIB_MAX_FILENAME);
	
	FILE *fp = fopen(szPath,"rb"); 
	if(fp == NULL){
		cerr << strerror(errno) << std::endl;
		return CLIB_ERROR_FOPEN;
	}
 
	// ファイルサイズを調査
	fseek(fp,0,SEEK_END); 
	fgetpos(fp,&fsize); 
 
	fclose(fp);
 
	return (int)fsize;
}

// TDTファイル形式での画像ファイル書き出し。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_write_tdt(const char* pFileName, const unsigned short* pBuff, int  nMain, int  nSub)
{
	char szPath[CLIB_MAX_FILENAME];
	strncpy(szPath, pFileName, CLIB_MAX_FILENAME);
	strcat(szPath, ".tdt");

	// TDTファイルの書き出し
	int ret = clib_file_write(szPath, pBuff, sizeof(unsigned short) * nMain * nSub);
	if(ret < 0) return ret;

	// TMAファイルの書き出し
	ret = clib_write_tma(pFileName, nMain, nSub);
	if(ret < 0) return ret;

	return CLIB_OK;
}

// TMAヘッダのファイル書き出し。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_write_tma(const char* pFileName, int  nMain, int  nSub)
{
	char szPath[CLIB_MAX_FILENAME];
	strncpy(szPath, pFileName, CLIB_MAX_FILENAME);
	strcat(szPath, ".tma");

	// TMAファイルの書き出し
	char szTmaHeader[260];
	sprintf(szTmaHeader, "%05d%05d101000000000000                                                            0711061941366                                                                                                                                              -310100354004740", nMain, nSub);
	int ret = clib_file_write(szPath, szTmaHeader, strlen(szTmaHeader));
	if(ret < 0) return ret;

	return CLIB_OK;
}

// TDTファイルを読み込む。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_read_tdt(const char* pFileName, unsigned short* pBuff, int nMain, int nSub)
{
	int ActualMain, ActualSub;
	int ret = clib_read_tma(pFileName, ActualMain, ActualSub);
	if(ret != CLIB_OK) return ret;

	if((nMain != ActualMain) || (nSub != ActualSub)){
		return CLIB_WRONG_SIZE;
	}

	char szPath[CLIB_MAX_FILENAME];
	strncpy(szPath, pFileName, CLIB_MAX_FILENAME);
	strcat(szPath, ".tdt");

	// TDTファイルの読み込み
	ret = clib_file_read(szPath, pBuff, sizeof(unsigned short) * nMain * nSub);
	if(ret < 0) return ret;

	return CLIB_OK;
}

// TMAヘッダを読み込んで、画像のサイズを得る。
// 【注意】pFileNameには拡張子を含めないこと。
int clib_read_tma(const char* pFileName, int& nMain, int& nSub)
{
	char szPath[CLIB_MAX_FILENAME];
	strncpy(szPath, pFileName, CLIB_MAX_FILENAME);
	strcat(szPath, ".tma");

	// TMAファイルの書き出し
	char szTmaHeader[260];
	int ret = clib_file_read(szPath, szTmaHeader, sizeof(szTmaHeader));
	if(ret < 0) return ret;
	sscanf(szTmaHeader, "%05d%05d", &nMain, &nSub);

	return CLIB_OK;
}

// キャッシュ禁止の領域を確保する
void * clib_allocate_nocache(int size)
{
	return ::VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE | PAGE_NOCACHE);
}

// キャッシュ禁止の領域を開放する
bool clib_free_nocache(void *p)
{
	BOOL ret = ::VirtualFree(p, 0, MEM_RELEASE);

	if(ret == 0){
		cerr << "::VirtualFree() failed in clib_free_nocache()" << std::endl;
		return false;
	}
	return true;
}

#ifndef CLIB_USE_CPP_CLI
char* UnicodeToAnsi(const _TCHAR *pUnicode)
{
	char* result = NULL;
#ifdef UNICODE
	const int len = ::WideCharToMultiByte(CP_THREAD_ACP, 0, pUnicode, -1, NULL, 0, NULL, NULL);
	if (len > 0)
	{
		result = new char[len];
		if (!::WideCharToMultiByte(CP_THREAD_ACP, 0, pUnicode, -1, result, len, NULL, NULL))
		{
			delete[] result;
			result = NULL;
		}
	}
#else
	result = new char[source.GetLength() + 1];
	strcpy_s(result, source.GetLength() + 1, source);
#endif
	return result;
}
#endif

/*********************************************************************
	File Path	: TestPc\DefaultConfig\CommonLibPc.cpp
*********************************************************************/

