#pragma once

#include <string>

class IConsoleOut
{
public:
	virtual void WriteLineStdOut(const char* sz) = 0;
	virtual void WriteLineStdErr(const char* sz) = 0;

	virtual void WriteStdOut(const char* sz) = 0;
	virtual void WriteStdErr(const char* sz) = 0;

	void WriteLineStdOutString(const std::string& str) { this->WriteLineStdOut(str.c_str()); }
	void WriteLineStdErrString(const std::string& str) { this->WriteLineStdErr(str.c_str()); }

	void WriteStdOutString(const std::string& str) { this->WriteStdOut(str.c_str()); }
	void WriteStdErrString(const std::string& str) { this->WriteStdErr(str.c_str()); }
};
