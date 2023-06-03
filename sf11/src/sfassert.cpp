#include "sfassert.h"
#include "d3d11_include.h"
#include "comdef.h"
#include <string>

void sf11::sfAssert(bool condition, const char* message)
{
	if (!condition)
	{
		MessageBox(NULL, message, "Assertion Failed", 0);
		throw;
		exit(1);
	}
}
 
void sf11::sfAssertHR(HRESULT hr, const char* message)
{
	if (!SUCCEEDED(hr))
	{
		_com_error err(hr);
		LPCTSTR hrMsg = err.ErrorMessage();
		std::string str1(message);
		std::string str2(hrMsg);
		MessageBoxA(NULL, std::string(str1 + "\n" + str2).c_str(), "Assertion Failed", MB_OK);
		throw;
		exit(1);
	}
}
