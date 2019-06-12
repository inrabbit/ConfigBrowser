#include "stdafx.h"
#include "CommonDotNet.h"

/* 
 * to_cli: std::string ¨ System::String^
 */
System::String^ to_cli(const std::string& str)
{
	return gcnew System::String(str.c_str());
}

/* 
 * from_cli: System::String^ ¨ std::string
 */
std::string from_cli(System::String^ str)
{
	System::Text::Encoding^ sjis = System::Text::Encoding::GetEncoding( "Shift_JIS" );
	int nCount = sjis->GetByteCount( str );
	std::string std_string; // Shift-JIS•ÏŠ·Œã‚Ìchar*”z—ñ‚ª“ü‚é
	for(int i = 0 ; i < nCount ; i++){
		std_string.push_back( sjis->GetBytes(str)[i] );
	}

	return std_string;
}
