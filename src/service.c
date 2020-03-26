#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "service.h"

PCHAR ExtractService(
	PCHAR szDriverPath
)
{
	PCHAR szDriverSvc = NULL;
	PCHAR szDriverStr = NULL;
	PCHAR szDriverExt = NULL;
	DWORD dwDriverLen = 0;

	if ( ((szDriverStr = strrchr(szDriverPath, '\\')) != NULL) ||
	     ((szDriverStr = strrchr(szDriverPath, '/'))  != NULL) &&
	     ((szDriverExt = strstr(szDriverPath, ".sys")) != NULL)
	   )
	{
		szDriverStr++;

		dwDriverLen = (DWORD)(szDriverExt - szDriverStr);
		szDriverSvc = malloc(dwDriverLen+1);

		strncpy(szDriverSvc, szDriverStr, dwDriverLen);

		return szDriverSvc;
	};
	return NULL;
};

BOOL LoadDriver(
	PCHAR szDriverPath
)
{
  PCHAR     szDriverSvc  = NULL;
  SC_HANDLE ServiceMan   = NULL;
  SC_HANDLE ServicePtr   = NULL;
  BOOL      boolRetVal   = FALSE;

  szDriverSvc = ExtractService(szDriverPath);
  ServiceMan  = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
  if ( ServiceMan != NULL ) {
    ServicePtr = CreateServiceA(ServiceMan, szDriverSvc, szDriverSvc,
		    SERVICE_START | DELETE | SERVICE_STOP,
		    SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
		    SERVICE_ERROR_IGNORE, szDriverPath, NULL,
		    NULL, NULL, NULL, NULL);
    if ( ServicePtr != NULL ) {
	printf("[ ] registered service %s successfully\n", szDriverSvc);
	if ( StartService(ServicePtr, 0, NULL) == TRUE ) {
		printf("[ ] started service %s successfully\n", szDriverSvc);
		boolRetVal = TRUE;
	} else { printf("[!] LoadDriver(): GetLastError() -> 0x%x\n", GetLastError()); };
	CloseServiceHandle(ServicePtr);
    } else { printf("[!] LoadDriver(): GetLastError() -> 0x%x\n", GetLastError()); };
    CloseServiceHandle(ServiceMan);
  };
  if ( szDriverSvc != NULL )
	  free(szDriverSvc);

  return boolRetVal;
};

BOOL UnloadDriver(
	PCHAR szDriverPath
)
{
  PCHAR          szDriverSvc  = NULL;
  SC_HANDLE      ServiceMan   = NULL;
  SC_HANDLE      ServicePtr   = NULL;
  SERVICE_STATUS ServiceStat  =  { 0 };
  BOOL           boolRetVal   = FALSE;

  szDriverSvc = ExtractService(szDriverPath);
  ServiceMan  = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
  if ( ServiceMan != NULL ) {
	ServicePtr = OpenServiceA(ServiceMan, szDriverSvc, SERVICE_STOP | DELETE);
	if ( ServicePtr != NULL ) {
		ControlService(ServicePtr, SERVICE_CONTROL_STOP, &ServiceStat);
		if ( DeleteService(ServicePtr) != TRUE ) {
			printf("[!] failed to delete service %s, cleanup manually!\n", szDriverSvc);
		} else { printf("[ ] deleted service %s successfully\n", szDriverSvc); };
		CloseServiceHandle(ServicePtr);
		boolRetVal = TRUE;
	};
	CloseServiceHandle(ServiceMan);
  };
  if ( szDriverSvc != NULL )
	  free(szDriverSvc);

  return boolRetVal;
};
