#include "StorageAlignmentInfo.h"
#include <stdexcept>

// ---------------------------------------------------------------------------------------------
// The following definitions are from:
// 
// -> https://github.com/joyent/libuv/tree/0aee1ec7b14dc37d02a9a3fea2a2a06df77acb0c/src/win
//    Files winapi.h / winapi.cpp
//
// https://docs.microsoft.com/en-us/windows/win32/w8cookbook/advanced-format--4k--disk-compatibility-update?redirectedfrom=MSDN

typedef struct _IO_STATUS_BLOCK
{
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } DUMMYUNIONNAME;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;


typedef enum _FS_INFORMATION_CLASS
{
    FileFsVolumeInformation = 1,
    FileFsLabelInformation = 2,
    FileFsSizeInformation = 3,
    FileFsDeviceInformation = 4,
    FileFsAttributeInformation = 5,
    FileFsControlInformation = 6,
    FileFsFullSizeInformation = 7,
    FileFsObjectIdInformation = 8,
    FileFsDriverPathInformation = 9,
    FileFsVolumeFlagsInformation = 10,
    FileFsSectorSizeInformation = 11
} FS_INFORMATION_CLASS, * PFS_INFORMATION_CLASS;

typedef struct _FILE_FS_SECTOR_SIZE_INFORMATION {
    ULONG LogicalBytesPerSector;
    ULONG PhysicalBytesPerSectorForAtomicity;
    ULONG PhysicalBytesPerSectorForPerformance;
    ULONG FileSystemEffectivePhysicalBytesPerSectorForAtomicity;
    ULONG Flags;
    ULONG ByteOffsetForSectorAlignment;
    ULONG ByteOffsetForPartitionAlignment;
} FILE_FS_SECTOR_SIZE_INFORMATION, * PFILE_FS_SECTOR_SIZE_INFORMATION;


typedef struct _FILE_FS_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER AvailableAllocationUnits;
    ULONG         SectorsPerAllocationUnit;
    ULONG         BytesPerSector;
} FILE_FS_SIZE_INFORMATION, * PFILE_FS_SIZE_INFORMATION;

typedef NTSTATUS(NTAPI* sNtQueryVolumeInformationFile)
(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FsInformation, ULONG Length, FS_INFORMATION_CLASS FsInformationClass);

static sNtQueryVolumeInformationFile pNtQueryVolumeInformationFile = nullptr;

// ---------------------------------------------------------------------------------------------

static bool EnsureQueryVolumeInformationFile()
{
    if (pNtQueryVolumeInformationFile == nullptr)
    {
        HMODULE hMod = GetModuleHandleA("ntdll.dll");
        pNtQueryVolumeInformationFile = (sNtQueryVolumeInformationFile)GetProcAddress(hMod, "NtQueryVolumeInformationFile");
    }

    return (pNtQueryVolumeInformationFile != nullptr) ? true : false;
}

StorageAlignmentInfo QueryStorageAlignmentInfo(HANDLE hFile)
{
    if (!EnsureQueryVolumeInformationFile())
    {
        throw std::runtime_error("Couldn't get \"NtQueryVolumeInformationFile\"-API.");
    }

    // structure documented at https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/ns-ntddk-_file_fs_sector_size_information
    FILE_FS_SECTOR_SIZE_INFORMATION ffssi;
    IO_STATUS_BLOCK iosb;
    NTSTATUS status = pNtQueryVolumeInformationFile(hFile, &iosb, &ffssi, sizeof(ffssi), FileFsSectorSizeInformation);
    if (status != 0/*STATUS_SUCCESS*/)
    {
        throw std::runtime_error("\"NtQueryVolumeInformationFile\" failed.");
    }

    StorageAlignmentInfo info;
    info.bytesPerPhysicalSector = ffssi.PhysicalBytesPerSectorForPerformance;

    return info;
}
