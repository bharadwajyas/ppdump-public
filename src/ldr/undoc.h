#ifndef _UNDOC_H_
#define _UNDOC_H_

typedef struct _C_PEB_LDR_DATA
{
	ULONG Length;
	BOOL Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
} C_PEB_LDR_DATA, *C_PPEB_LDR_DATA;

typedef struct _C_LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID Entrypoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
} C_LDR_DATA_TABLE_ENTRY, *C_PLDR_DATA_TABLE_ENTRY;

#endif
