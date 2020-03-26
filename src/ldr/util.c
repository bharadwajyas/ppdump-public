#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include "undoc.h"
#include "hash.h"
#define PTR(x) ((ULONG_PTR)x)
#define NT_HEADER(x) ( PTR(x) + ((PIMAGE_DOS_HEADER)x)->e_lfanew )

LPVOID GetLoadedModule( _In_ DWORD ModuleHash )
{
	PLIST_ENTRY             LstEnt = NULL;
	PLIST_ENTRY             LstHdr = NULL;
	PPEB                    PebPtr = NULL;
	C_PPEB_LDR_DATA         PebLdr = NULL;
	C_PLDR_DATA_TABLE_ENTRY LdrEnt = NULL; 
	DWORD                   HshMod = 0;

#if defined(_M_X64) || defined(_WIN64)
	PebPtr = (PPEB)__readgsqword(0x60);
#else
	PebPtr = (PPEB)__readfsdword(0x30);
#endif

	PebLdr = (C_PPEB_LDR_DATA)PebPtr->Ldr;
	LstHdr = &PebLdr->InLoadOrderModuleList;
	LstEnt = LstHdr->Flink;
	while ( ( LstEnt != LstHdr ) && ( LstEnt = LstEnt->Flink ) ) {
		LdrEnt = (C_PLDR_DATA_TABLE_ENTRY)LstEnt;
		HshMod = HashStringDjb2(LdrEnt->BaseDllName.Buffer,
								LdrEnt->BaseDllName.Length);
		if ( HshMod == ModuleHash ) return LdrEnt->DllBase;
	};
	return NULL;
};

LPVOID GetMemoryFunction( _In_ LPVOID ModuleBase, _In_ DWORD FunctionHash )
{
	PIMAGE_NT_HEADERS       NtsHdr = NULL;
	PIMAGE_EXPORT_DIRECTORY ExpHdr = NULL;
	PIMAGE_DATA_DIRECTORY   DatHdr = NULL;
	DWORD                   FunHsh = 0;
	PDWORD                  StrOff = NULL;
	PDWORD                  FunOff = NULL;
	PUSHORT                 OrdOff = NULL;
	SIZE_T                  nCount = 0;

	NtsHdr = (PIMAGE_NT_HEADERS)NT_HEADER( ModuleBase );
	DatHdr = &NtsHdr->OptionalHeader.DataDirectory[0];
	ExpHdr = (PIMAGE_EXPORT_DIRECTORY)(PTR(ModuleBase) + 
			 DatHdr->VirtualAddress);

	StrOff = (PDWORD) (PTR( ModuleBase ) + ExpHdr->AddressOfNames);
	FunOff = (PDWORD) (PTR( ModuleBase ) + ExpHdr->AddressOfFunctions);
	OrdOff = (PUSHORT)(PTR( ModuleBase ) + ExpHdr->AddressOfNameOrdinals);

	for ( int i = 0 ; i < ExpHdr->NumberOfNames ; i++ )
	{
		FunHsh = HashStringDjb2( (PCHAR)( PTR( ModuleBase ) + StrOff[i] ),
	                             (DWORD)( 0 ) );
		if ( FunHsh == FunctionHash ) return (LPVOID)( 
			PTR(ModuleBase) + FunOff[OrdOff[i]] 
		);
	};
	return NULL;
};

void InlineCopyMemory( _In_ LPVOID PtrOne, _In_ LPVOID PtrTwo, SIZE_T Length )
{
	SIZE_T LengthToCopy = Length;
	LPVOID CopyTo       = PtrOne;
	LPVOID CopyFrom     = PtrTwo;

	do 
	{
		*(BYTE *)CopyTo++ = *(BYTE *)CopyFrom++;
	} while ( LengthToCopy-- != 0 );
};
