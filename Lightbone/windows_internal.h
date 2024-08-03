#pragma once
#pragma warning(push)
#pragma warning(disable:4005)
#include <phnt_windows.h>
#include <phnt.h>
#pragma warning(pop)

#include <stdint.h>
#include<type_traits>

template <typename T>
struct _LIST_ENTRY_T
{
    T Flink;
    T Blink;
};

template <typename T>
struct _UNICODE_STRING_T
{
    using type = T;

    uint16_t Length;
    uint16_t MaximumLength;
    T Buffer;
};

template <int n>
using const_int = std::integral_constant<int, n>;

template<typename T>
constexpr bool is32bit = std::is_same_v<T, uint32_t>;

template<typename T, int v32, int v64>
constexpr int int_32_64 = std::conditional_t<is32bit<T>, const_int<v32>, const_int<v64>>::value;

template<typename T>
struct _PEB_T
{
    static_assert(std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>, "T must be uint32_t or uint64_t");

    uint8_t InheritedAddressSpace;
    uint8_t ReadImageFileExecOptions;
    uint8_t BeingDebugged;
    union
    {
        uint8_t BitField;
        struct
        {
            uint8_t ImageUsesLargePages : 1;
            uint8_t IsProtectedProcess : 1;
            uint8_t IsImageDynamicallyRelocated : 1;
            uint8_t SkipPatchingUser32Forwarders : 1;
            uint8_t IsPackagedProcess : 1;
            uint8_t IsAppContainer : 1;
            uint8_t IsProtectedProcessLight : 1;
            uint8_t SpareBits : 1;
        };
    };
    T Mutant;
    T ImageBaseAddress;
    T Ldr;
    T ProcessParameters;
    T SubSystemData;
    T ProcessHeap;
    T FastPebLock;
    T AtlThunkSListPtr;
    T IFEOKey;
    union
    {
        T CrossProcessFlags;
        struct
        {
            uint32_t ProcessInJob : 1;
            uint32_t ProcessInitializing : 1;
            uint32_t ProcessUsingVEH : 1;
            uint32_t ProcessUsingVCH : 1;
            uint32_t ProcessUsingFTH : 1;
            uint32_t ReservedBits0 : 27;
        };
    };
    union
    {
        T KernelCallbackTable;
        T UserSharedInfoPtr;
    };
    uint32_t SystemReserved;
    uint32_t AtlThunkSListPtr32;
    T ApiSetMap;
    union
    {
        uint32_t TlsExpansionCounter;
        T Padding2;
    };
    T TlsBitmap;
    uint32_t TlsBitmapBits[2];
    T ReadOnlySharedMemoryBase;
    T SparePvoid0;
    T ReadOnlyStaticServerData;
    T AnsiCodePageData;
    T OemCodePageData;
    T UnicodeCaseTableData;
    uint32_t NumberOfProcessors;
    uint32_t NtGlobalFlag;
    LARGE_INTEGER CriticalSectionTimeout;
    T HeapSegmentReserve;
    T HeapSegmentCommit;
    T HeapDeCommitTotalFreeThreshold;
    T HeapDeCommitFreeBlockThreshold;
    uint32_t NumberOfHeaps;
    uint32_t MaximumNumberOfHeaps;
    T ProcessHeaps;
    T GdiSharedHandleTable;
    T ProcessStarterHelper;
    union
    {
        uint32_t GdiDCAttributeList;
        T Padding3;
    };
    T LoaderLock;
    uint32_t OSMajorVersion;
    uint32_t OSMinorVersion;
    uint16_t OSBuildNumber;
    uint16_t OSCSDVersion;
    uint32_t OSPlatformId;
    uint32_t ImageSubsystem;
    uint32_t ImageSubsystemMajorVersion;
    union
    {
        uint32_t ImageSubsystemMinorVersion;
        T Padding4;
    };
    T ActiveProcessAffinityMask;
    uint32_t GdiHandleBuffer[int_32_64<T, 34, 60>];
    T PostProcessInitRoutine;
    T TlsExpansionBitmap;
    uint32_t TlsExpansionBitmapBits[32];
    union
    {
        uint32_t SessionId;
        T Padding5;
    };
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    T pShimData;
    T AppCompatInfo;
    _UNICODE_STRING_T<T> CSDVersion;
    T ActivationContextData;
    T ProcessAssemblyStorageMap;
    T SystemDefaultActivationContextData;
    T SystemAssemblyStorageMap;
    T MinimumStackCommit;
    T FlsCallback;
    _LIST_ENTRY_T<T> FlsListHead;
    T FlsBitmap;
    uint32_t FlsBitmapBits[4];
    uint32_t FlsHighIndex;
    T WerRegistrationData;
    T WerShipAssertPtr;
    T pUnused;
    T pImageHeaderHash;
    union
    {
        uint64_t TracingFlags;
        struct
        {
            uint32_t HeapTracingEnabled : 1;
            uint32_t CritSecTracingEnabled : 1;
            uint32_t LibLoaderTracingEnabled : 1;
            uint32_t SpareTracingBits : 29;
        };
    };
    T CsrServerReadOnlySharedMemoryBase;
};

template<typename T>
struct _PEB_LDR_DATA2_T
{
    uint32_t Length;
    uint8_t Initialized;
    T SsHandle;
    _LIST_ENTRY_T<T> InLoadOrderModuleList;
    _LIST_ENTRY_T<T> InMemoryOrderModuleList;
    _LIST_ENTRY_T<T> InInitializationOrderModuleList;
    T EntryInProgress;
    uint8_t ShutdownInProgress;
    T ShutdownThreadId;
};

template<typename T>
struct _PROCESS_BASIC_INFORMATION_T;

template<>
struct _PROCESS_BASIC_INFORMATION_T<uint64_t>
{
    NTSTATUS    ExitStatus;
    uint32_t    Reserved0;
    uint64_t	PebBaseAddress;
    uint64_t	AffinityMask;
    LONG	    BasePriority;
    ULONG	    Reserved1;
    uint64_t    uUniqueProcessId;
    uint64_t	uInheritedFromUniqueProcessId;
};

template<>
struct _PROCESS_BASIC_INFORMATION_T<uint32_t>
{
    NTSTATUS ExitStatus;
    UINT32 PebBaseAddress;
    UINT32 AffinityMask;
    UINT32 BasePriority;
    UINT32 UniqueProcessId;
    UINT32 InheritedFromUniqueProcessId;
};

template<typename T>
struct _LDR_DATA_TABLE_ENTRY_BASE_T
{
    _LIST_ENTRY_T<T> InLoadOrderLinks;
    _LIST_ENTRY_T<T> InMemoryOrderLinks;
    _LIST_ENTRY_T<T> InInitializationOrderLinks;
    T DllBase;
    T EntryPoint;
    uint32_t SizeOfImage;
    _UNICODE_STRING_T<T> FullDllName;
    _UNICODE_STRING_T<T> BaseDllName;
    uint32_t Flags;
    uint16_t LoadCount;
    uint16_t TlsIndex;
    _LIST_ENTRY_T<T> HashLinks;
    uint32_t TimeDateStamp;
    T EntryPointActivationContext;
    T PatchInformation;
};

template <typename T>
struct _CLIENT_ID_T
{
    T UniqueProcess;
    T UniqueThread;
};

template<typename T>
struct _SYSTEM_THREAD_INFORMATION_T
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    uint32_t WaitTime;
    T StartAddress;
    _CLIENT_ID_T<T> ClientId;
    LONG Priority;
    LONG BasePriority;
    uint32_t ContextSwitches;
    uint32_t ThreadState;
    uint32_t WaitReason;
};

template<typename T>
struct _SYSTEM_EXTENDED_THREAD_INFORMATION_T
{
    _SYSTEM_THREAD_INFORMATION_T<T> ThreadInfo;
    T StackBase;
    T StackLimit;
    T Win32StartAddress;
    T TebBase;
    T Reserved[3];
};

template<typename T>
struct _VM_COUNTERS_T
{
    T PeakVirtualSize;
    T VirtualSize;
    uint32_t PageFaultCount;
    T PeakWorkingSetSize;
    T WorkingSetSize;
    T QuotaPeakPagedPoolUsage;
    T QuotaPagedPoolUsage;
    T QuotaPeakNonPagedPoolUsage;
    T QuotaNonPagedPoolUsage;
    T PagefileUsage;
    T PeakPagefileUsage;
};


template<typename T>
struct _SYSTEM_PROCESS_INFORMATION_T
{
    uint32_t NextEntryOffset;
    uint32_t NumberOfThreads;
    LARGE_INTEGER WorkingSetPrivateSize;
    uint32_t HardFaultCount;
    uint32_t NumberOfThreadsHighWatermark;
    ULONGLONG CycleTime;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    _UNICODE_STRING_T<T> ImageName;
    LONG BasePriority;
    T UniqueProcessId;
    T InheritedFromUniqueProcessId;
    uint32_t HandleCount;
    uint32_t SessionId;
    T UniqueProcessKey;
    _VM_COUNTERS_T<T> VmCounters;
    T PrivatePageCount;
    IO_COUNTERS IoCounters;
    _SYSTEM_EXTENDED_THREAD_INFORMATION_T<T> Threads[1];
};

NTSTATUS NTAPI NtWow64QueryInformationThread64(_In_ HANDLE ThreadHandle,
    _In_ ULONG ThreadInformationClass,
    _Out_writes_bytes_(ThreadInformationLength) PVOID ThreadInformation,
    _In_ ULONG ThreadInformationLength,
    _Out_opt_ PULONG ReturnLength);

BOOL WINAPI CreateProcessInternalW(
    HANDLE hToken, LPCWSTR AppName, LPWSTR CmdLine,
    LPSECURITY_ATTRIBUTES ProcessAttr, LPSECURITY_ATTRIBUTES ThreadAttr,
    BOOL bIH, DWORD flags, LPVOID env, LPCWSTR CurrDir, LPSTARTUPINFOW si,
    LPPROCESS_INFORMATION pi, PHANDLE NewToken);