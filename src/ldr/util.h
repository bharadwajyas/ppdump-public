#ifndef _UTIL_H_
#define _UTIL_H_

LPVOID GetLoadedModule( _In_ DWORD ModuleHash );

LPVOID GetMemoryFunction( _In_ LPVOID ModuleBase, _In_ DWORD FunctionHash );

void InlineCopyMemory( _In_ LPVOID PtrOne, _In_ LPVOID PtrTwo, SIZE_T Length );

#endif
