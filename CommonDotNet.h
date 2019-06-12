#ifndef DEF_COMMON_DOTNET
#define DEF_COMMON_DOTNET

#include <string>

/* 
 * to_cli: std::string Å® System::String^
 */
System::String^ to_cli(const std::string& str);

/* 
 * from_cli: System::String^ Å® std::string
 */
std::string from_cli(System::String^ str);

#endif
