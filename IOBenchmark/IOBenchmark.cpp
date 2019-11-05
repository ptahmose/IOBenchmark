// IOBenchmark.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include "utf8convert.h"
#include "cmdlineargs.h"
#include "IWriter.h"
#include "WriterBasic.h"
#include "timeit.h"
#include "WinConsoleOut.h"

using namespace std;

int main()
{
	unique_ptr<WinConsoleOut> conout(new WinConsoleOut);

	int argc;
	LPWSTR* szarglist = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::vector<string> utf8Args;
	utf8Args.reserve(argc);
	for (int i = 0; i < argc; ++i)
	{
		utf8Args.push_back(Utf16ToUtf8(wstring(szarglist[i])));
	}

	char** argv = new char* [argc];
	for (int i = 0; i < argc; ++i)
	{
		argv[i] = (char*)(utf8Args[i].c_str());
	}

	CCmdlineArgs cmdlineArgs;
	cmdlineArgs.ParseArguments(argc, argv);

	stringstream ss;
	ss << "Outputfile is \"" << cmdlineArgs.GetFilename() << "\".";
	conout->WriteLineStdOutString(ss.str());

	shared_ptr<IWriter> writer = IWriter::CreateInstance(cmdlineArgs.GetWriterType());

	IWriter::WriterOptions writerOptions;
	writerOptions.blkSize = cmdlineArgs.GetBlkSize();
	writerOptions.fileSize = cmdlineArgs.GetFileSize();
	writerOptions.filename = cmdlineArgs.GetFilename();
	try
	{
		writer->Init(writerOptions);
	}
	catch (WriterException & excp)
	{
		stringstream ss;
		ss << "Error when initializing the writer -> " << excp.what();
		conout->WriteLineStdErrString(ss.str());
		if (excp.GetIsValidHresult())
		{
			ss = stringstream();
			ss << "OS-Error: " << excp.GetOSError();
			conout->WriteLineStdErrString(ss.str());
		}

		return -1;
	}

	CTimeIt timeit;
	try
	{
		writer->DoIt();
	}
	catch (WriterException & excp)
	{
		excp.GetOSError();
	}

	timeit.Stop();

	uint64_t byteswritten = (writerOptions.fileSize / writerOptions.blkSize) * writerOptions.blkSize;

	cout << "data rate is " << byteswritten / timeit.GetElapsedTimeInSeconds() / 1000000.0 << "MB/s";
}
