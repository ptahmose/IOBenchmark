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

	conout->WriteLineStdOutString("Another line");

	shared_ptr<IWriter> writer = IWriter::CreateInstance(IWriter::WriterType::Async);

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
		excp.GetOSError();
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

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
