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
	// データから文字列に変換
	// *********************************************
	// 型情報サフィックス付
	const char *ToString(const UnivType& x);
	// 型情報サフィックスなし
	const char *ToStringWithoutSuffix(const UnivType& x);

	// *********************************************
	// 文字列からデータに変換
	// *********************************************
	// 文字列型とバイナリ型・トークン中に改行が入るもの（終端NULLに限定）
	UnivType& ToUnivType(const char *pString);
	// 文字列型とバイナリ型・トークン中に改行が入るもの（終端位置をポインタで指定）
	UnivType& ToUnivType(const char *pStringBegin, const char *pStringEnd);
	// 文字列型とバイナリ型以外・トークン中に改行が入らないもの（終端位置を検出して返す）
	UnivType& ToUnivTypeWithoutNewLine(const char *pStringBegin, const char *&pStringEnd);
	// 文字列型とバイナリ型以外・トークン中に改行が入らないもの（終端位置を返さない）
	UnivType& ToUnivTypeWithoutNewLine(const char *pStringBegin);
};

#endif
