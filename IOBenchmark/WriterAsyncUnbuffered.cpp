#include "WriterAsyncUnbuffered.h"

using namespace std;

//---------------------------------------------------------------------------------------

WriterAsyncUnbuffered::WriterAsyncUnbuffered() :
    hFile(INVALID_HANDLE_VALUE)
{
}

/*virtual*/void WriterAsyncUnbuffered::Init(const WriterOptions& options, std::shared_ptr<IPropertyBagRead> writerSpecificOptions)
{
  /*  int maxPendingOperationsCount = DefaultMaxPendingOperationCount;
    if (writerSpecificOptions)
    {
        if (writerSpecificOptions->TryGetInt("MaxPendingOperations", &maxPendingOperationsCount) == true)
        {
            if (maxPendingOperationsCount <= 0)
            {
                throw invalid_argument("Parameter 'MaxPendingOperations' must be greater than zero.");
            }
        }
    }*/

    const auto filenameW = Utf8ToUtf16(options.filename);
    HANDLE h = CreateFileW(
        filenameW.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        stringstream ss;
        ss << "Error when calling \"CreateFile\" with filename \"" << options.filename << "\".";
        auto excp = WriterException(WriterException::ErrorType::APIError, ss.str());;
        excp.SetLastError(GetLastError());
        throw excp;
    }

    this->DetermineAlignmentInformation(h,filenameW.c_str());
}

/*virtual*/void WriterAsyncUnbuffered::DoIt()
{}

/*virtual*/WriterAsyncUnbuffered::~WriterAsyncUnbuffered()
{}


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
(HANDLE FileHandle,PIO_STATUS_BLOCK IoStatusBlock,PVOID FsInformation,ULONG Length,FS_INFORMATION_CLASS FsInformationClass);


void WriterAsyncUnbuffered::DetermineAlignmentInformation(HANDLE hFile,LPCWSTR szwFilename)
{
    // -> https://github.com/joyent/libuv/tree/0aee1ec7b14dc37d02a9a3fea2a2a06df77acb0c/src/win
    //    Files winapi.h / winapi.cpp
    //
    // https://docs.microsoft.com/en-us/windows/win32/w8cookbook/advanced-format--4k--disk-compatibility-update?redirectedfrom=MSDN

    HMODULE hMod = GetModuleHandleA("ntdll.dll");
    
    sNtQueryVolumeInformationFile pNtQueryVolumeInformationFile = (sNtQueryVolumeInformationFile)GetProcAddress(hMod, "NtQueryVolumeInformationFile");
    if (hFile != INVALID_HANDLE_VALUE)
    {
        FILE_FS_SECTOR_SIZE_INFORMATION ffssi;
        FILE_FS_SIZE_INFORMATION ffsi;

        IO_STATUS_BLOCK iosb;

        pNtQueryVolumeInformationFile(hFile, &iosb, &ffsi, sizeof(ffsi), FileFsSizeInformation);
        pNtQueryVolumeInformationFile(hFile, &iosb, &ffssi, sizeof(ffssi), FileFsSectorSizeInformation);
        CloseHandle(hFile);

    }








    WCHAR volPathName[244];
    BOOL B = GetVolumePathNameW(
        szwFilename,
        volPathName,
        sizeof(volPathName) / sizeof(volPathName[0]));

    HANDLE hDevice = CreateFileW(
        /*volPathName*/L"\\\\.\\C:)",
        0,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        return;
    }

    // Now that we have the device handle for the disk, let us get disk's metadata
    DWORD outsize;
    STORAGE_PROPERTY_QUERY storageQuery;
    memset(&storageQuery, 0, sizeof(STORAGE_PROPERTY_QUERY));
    storageQuery.PropertyId = StorageAccessAlignmentProperty;
    storageQuery.QueryType = PropertyStandardQuery;

    STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR diskAlignment = { 0 };
    memset(&diskAlignment, 0, sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR));

    if (!DeviceIoControl(hDevice,
        IOCTL_STORAGE_QUERY_PROPERTY,
        &storageQuery,
        sizeof(STORAGE_PROPERTY_QUERY),
        &diskAlignment,
        sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR),
        &outsize,
        NULL)
        )
    {
        return;
    }
}