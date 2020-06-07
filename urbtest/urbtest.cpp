// urbtest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "utils.h"

#include "../urb/UnbufferedFileWriter.h"
#include "../urb/UnbufferedFileWriter2.h"

int main()
{
    std::cout << "Hello World!\n";

    std::unique_ptr<void, void(*)(void*)> _64k(malloc(64 * 1024), &free);
    FillWithAbc(_64k.get(), 64 * 1024);
    std::unique_ptr<void, void(*)(void*)> _128(malloc(128), &free);
    memset(_128.get(), 'X', 128);

   /* CUnbufferedFileWriter writer;
    writer.InitializeFile(L"D:\\test.bin");

    for (int i = 0; i < 32; ++i)
    {
        writer.TryAppendNoWait(_64k.get(), 64 * 1024);
    }

    Sleep(400);
    writer.Rewrite(3, _128.get(), 5);
    writer.Close();*/
    CUnbufferedFileWriter2 writer;
    writer.InitializeFile(L"D:\\test.bin");
    bool b;
    for (int i = 0; i < 16; ++i)
    {
        writer.TryAppendNoWait(i*64*1024,_64k.get(), 64 * 1024);
    }

    writer.TryAppendNoWait(16 * 64 * 1024, _128.get(), 1);

    Sleep(100);
    for (int i = 0; i < 16; ++i)
    {
        for (;;)
        {
            b = writer.TryAppendNoWait((16 + i) * 64 * 1024 + 1, _64k.get(), 64 * 1024);
            if (b) break;
            Sleep(100);
        }
    }



    writer.Close();
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
