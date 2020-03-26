#define _GNU_SOURCE
#include <windows.h>
#include <winternl.h>
#include <string.h>
#include <stdio.h>

typedef
NTSTATUS
( NTAPI * fnRtlRandomEx )
(
	_In_ _Out_ PULONG Seed
);

BOOL DumpBufferToFile( _In_ LPCSTR lpszFilePath,
                       _In_ LPVOID lpBufferToDump,
                       _In_ SIZE_T LengthOfBuffer )
{
	HANDLE hFile       = NULL;
	DWORD  LengthWrote = 0;

	hFile = CreateFileA( lpszFilePath, GENERIC_READ | GENERIC_WRITE,
                             0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                             NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		WriteFile( hFile, lpBufferToDump, LengthOfBuffer, &LengthWrote, NULL );
		if ( LengthWrote == LengthOfBuffer ) {
			printf("[+] Wrote %lu to %s successfully\n", LengthWrote, lpszFilePath);
		} else {
			printf("[!] DumpBufferToFile(): GetLastError() -> 0x%x\n", GetLastError());
		}; CloseHandle( hFile );
		return TRUE;
	} else { printf("[!] DumpBufferToFile(): GetLastError -> 0x%x\n", GetLastError()); };

	return FALSE;
};

PCHAR GetTemporaryFile( _In_ INT MaxFileName, _In_ PCHAR szExtension )
{
	CHAR  szTempPath[MAX_PATH + 1];
	CHAR  szFileName[MaxFileName + 1];
	PCHAR szFullFilePath              = NULL;
	PCHAR szBadChar                   = NULL;
	ULONG ulSeedCount                 = 0;
	ULONG ulIndexTbl                  = 0;
	fnRtlRandomEx RtlRandomEx         = NULL;
	CHAR  szAnsiTable[]               =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz";

	RtlSecureZeroMemory( &szTempPath, MAX_PATH + 1);
	RtlSecureZeroMemory( &szFileName, MaxFileName + 1);
	if ( GetTempPathA( MAX_PATH + 1, (PCHAR)&szTempPath ) != 0 ) {
		ulSeedCount = GetTickCount();

		for ( int i = 0 ; i < MaxFileName ; i++ ) {
			RtlRandomEx   = (fnRtlRandomEx)GetProcAddress( LoadLibraryA("ntdll.dll"), "RtlRandomEx" );
			ulIndexTbl    = RtlRandomEx(&ulSeedCount) % (sizeof(szAnsiTable) - 2);
			szFileName[i] = szAnsiTable[ulIndexTbl];
		};

		asprintf(&szFullFilePath, "%s%s%s", (PCHAR)&szTempPath, (PCHAR)&szFileName, szExtension);
		return szFullFilePath;
	};
	return NULL;
};
