/*
Module Name:user.exe

Abstract:
	This file contains the implementation for adding prerogative process to minifilter driver

Environment:
    User mode
*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winioctl.h>
#include <string.h>
#include <crtdbg.h>
#include <assert.h>
#include <fltuser.h>
#include <dontuse.h>
#include "../include/interface.h"

HANDLE port = NULL;
HRESULT hr = NULL;
void AddMonitorProcToKernel(char* ProcName)
{
	DWORD dwReturned;
	MSG_SEND_SET_PROCESS_INFO psSendSetProcInfo;
	MSG_SEND_ADD_PROCESS_INFO sSendAddProcInfo ;
	MSG_GET_ADD_PROCESS_INFO sGetAddProcInfo ;

	memset(&sSendAddProcInfo, 0, sizeof(sSendAddProcInfo)) ;
	sSendAddProcInfo.sSendType.uSendType = IOCTL_ADD_PROCESS_INFO ;
	sSendAddProcInfo.sProcInfo.bMonitor = TRUE;
	strncpy(sSendAddProcInfo.sProcInfo.szProcessName,ProcName, strlen(ProcName)) ;
	memset(&sGetAddProcInfo, 0, sizeof(MSG_GET_ADD_PROCESS_INFO)) ;
	hr = FilterSendMessage(port, &sSendAddProcInfo, sizeof(MSG_SEND_ADD_PROCESS_INFO), &sGetAddProcInfo, sizeof(MSG_GET_ADD_PROCESS_INFO), &dwReturned) ;

	if (IS_ERROR( hr )) {
		printf( "ERROR: SendMessage system: 0x%08x\n", hr );
	}
	if (sGetAddProcInfo.uResult == MGAPI_RESULT_SUCCESS)
	{
		printf("%s success\n",ProcName);		
	}
	else if (sGetAddProcInfo.uResult == MGAPI_RESULT_ALREADY_EXIST)
	{
		printf("%s has running\n",ProcName);		
	}
}


int _cdecl
main (
    __in int argc,
    __in_ecount(argc) char *argv[]
    )
{
	printf( "User.exe: Connecting to the filter ...\n" );
	hr = FilterConnectCommunicationPort( SERVER_PORTNAME,
		0, 
		NULL,
		0,
		NULL,
		&port );
	if (IS_ERROR( hr )) {
		printf( "ERROR: Connecting to filter port: 0x%08x\n", hr );
		return 2;
	}	
	
	AddMonitorProcToKernel("winword.exe");
	AddMonitorProcToKernel("excel.exe");
	AddMonitorProcToKernel("powerpnt.exe");
	AddMonitorProcToKernel("wps.exe");
	AddMonitorProcToKernel("wpp.exe");
	AddMonitorProcToKernel("et.exe");	
	AddMonitorProcToKernel("System");//Because Find System operated office's file too. 
		
	CloseHandle( port );	
	
    return hr;
}