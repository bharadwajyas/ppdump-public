#ifndef _FSUTIL_H_
#define _FSUTIL_H_

PCHAR GetTemporaryFile( _In_ INT MaxFileName, _In_ PCHAR szExtension );
BOOL DumpBufferToFile( _In_ LPCSTR lpszFilePath,
                       _In_ LPVOID lpBufferToDump,
                       _In_ SIZE_T LengthOfBuffer );

#endif
