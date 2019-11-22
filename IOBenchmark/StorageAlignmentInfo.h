#pragma once
#include <Windows.h>

struct StorageAlignmentInfo
{
    DWORD bytesPerPhysicalSector;
};

StorageAlignmentInfo QueryStorageAlignmentInfo(HANDLE hFile);