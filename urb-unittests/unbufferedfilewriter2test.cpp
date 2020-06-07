#include "pch.h"
#include "include_urb.h"
#include "CFileApiMem.h"
#include "utils.h"

using namespace std;

TEST(UnbufferedFileWriter2, Test1)
{
    auto fileApiMem = make_unique<CFileApiMemImpl>();
    const CFileApiMemImpl* ptrFileApiMem = fileApiMem.get();

    CMd5Sum md5SumGen1; char md5sum1[16];
    CUnbufferedFileWriter2 fw(move(fileApiMem));
    fw.InitializeFile(L"xxx");

    std::unique_ptr<void, void(*)(void*)> _64k(malloc(64 * 1024), &free);
    FillWithAbc(_64k.get(), 64 * 1024);
    std::unique_ptr<void, void(*)(void*)> _128(malloc(128), &free);
    memset(_128.get(), 'X', 128);

    for (int i = 0; i < 16; ++i)
    {
        fw.TryAppendNoWait(i * 64 * 1024, _64k.get(), 64 * 1024);
        md5SumGen1.update(_64k.get(), 64 * 1024);
    }

    fw.TryAppendNoWait(16 * 64 * 1024, _128.get(), 1);
    md5SumGen1.update(_128.get(), 1);
    for (int i = 0; i < 16; ++i)
    {
        for (;;)
        {
            bool b = fw.TryAppendNoWait((16 + i) * 64 * 1024 + 1, _64k.get(), 64 * 1024);
            if (b)
            {
                md5SumGen1.update(_64k.get(), 64 * 1024);
                break;
            }

            Sleep(1);
        }
    }

    md5SumGen1.complete();
    md5SumGen1.getHash(md5sum1);

    fw.Close();

    const auto* ptr = ptrFileApiMem->GetDataPtr();
    const auto size = ptrFileApiMem->GetDataSize();
    CMd5Sum md5SumGen2; char md5sum2[16];
    md5SumGen2.update(ptr, size);
    md5SumGen2.complete();
    md5SumGen2.getHash(md5sum2);

    EXPECT_TRUE(memcmp(md5sum1, md5sum2, 16) == 0) << "The content of the filewriter is different than what we put into.";

    //CRingBufferManager bufMan(256, 4, nullptr);
    //char buf[1024];
    //bufMan.AddDataNonBlocking(buf, 255);
    //bufMan.AddDataNonBlocking(buf, 1);
    //EXPECT_EQ(1, 1);
    //EXPECT_TRUE(true);
}