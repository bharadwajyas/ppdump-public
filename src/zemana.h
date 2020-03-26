#ifndef _ZEMANA_H_
#define _ZEMANA_H_

HANDLE ZemanaOpenHandle();

BOOL ZemanaRegisterProcess( _In_ HANDLE hDevice,
                            _In_ DWORD TrustedProcess );
BOOL ZemanaOpenProcess( _In_ HANDLE hDevice,
                        _In_ DWORD ProcessNumber,
                        _Out_ PHANDLE ProcessHandle );
BOOL ZemanaOpenThread( _In_  HANDLE  hDevice,
                       _In_  DWORD   ThreadId,
                       _Out_ PHANDLE ThreadHandle );
#endif
