#pragma once
#include <string>
#include <wtypes.h>


#define CHECKNULLSTR(str) ((str) ? (str) : L"")
inline std::wstring interopString(BSTR bstr) {
    return std::wstring(CHECKNULLSTR(bstr), ::SysStringLen(bstr));
}