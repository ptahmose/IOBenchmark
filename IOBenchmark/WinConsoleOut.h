#include <Windows.h>
#include <iostream>
#include "IConsoleOut.h"
#include "utf8convert.h"

using namespace std;

class WinConsoleOut : public IConsoleOut
{
public:
	WinConsoleOut()
	{
	/*	DWORD l_mode;
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleMode(hStdout, &l_mode);
			SetConsoleMode(hStdout, l_mode |
				ENABLE_VIRTUAL_TERMINAL_PROCESSING |
				DISABLE_NEWLINE_AUTO_RETURN);*/
	}


	virtual void WriteLineStdOut(const char* sz)
	{
		//std::wcout << Utf8ToUtf16(sz) << endl;
		auto txt = Utf8ToUtf16(sz);
		DWORD charsWritten;
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), txt.c_str(), txt.size(), &charsWritten, NULL);
		WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\r\n",2, &charsWritten, NULL);
	}

	virtual void WriteLineStdErr(const char* sz)
	{
		std::wcout << Utf8ToUtf16(sz) << endl;
	}

	virtual void WriteStdOut(const char* sz)
	{
		std::wcout << Utf8ToUtf16(sz);
	}

	virtual void WriteStdErr(const char* sz)
	{
		std::wcout << Utf8ToUtf16(sz);
	}
};
