#include "pch.h"
#include "include_urb.h"

using namespace std;

TEST(RingBufferManager, AddDataNonBlocking1)
{
    CRingBufferManager bufMan(256, 4, nullptr);
    char buf[1024];
    bufMan.AddDataNonBlocking(buf, 255);
    bufMan.AddDataNonBlocking(buf, 1);
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}

TEST(RingBufferManager, AddDataNonBlocking2)
{
    bool expectedDataReceived = false;
    CRingBufferManager bufMan(256, 4,
        [&expectedDataReceived](const void* ptr, std::uint32_t size)->bool
        {
            if (size == 256)
            {
                bool allOk = true;
                for (size_t i = 0; i < size; ++i)
                {
                    if (*(((const uint8_t*)ptr) + i) != (uint8_t)i)
                    {
                        allOk = false;
                        break;
                    }
                }

                expectedDataReceived = allOk;
            }

            return true;
        });

    std::uint8_t buf[1024];
    for (size_t i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }

    bufMan.AddDataNonBlocking(buf, 255);
    bufMan.AddDataNonBlocking(buf+255, 2);

    bufMan.FinishAndShutdown();
    EXPECT_TRUE(expectedDataReceived) << "The test did not have the expected result.";
}