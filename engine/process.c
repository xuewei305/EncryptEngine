#include "process.h"
#include "..\include\iocommon.h"
#include "..\include\interface.h"

ULONG g_nProcessNameOffset = 0 ;
LIST_ENTRY g_ProcessListHead ;
KSPIN_LOCK g_ProcessListLock ;

static BOOLEAN Psi_SetProcessMonitor(PPROCESS_INFO psProcInfo, BOOLEAN bAll) ;
static BOOLEAN Psi_SearchForSpecifiedProcessInList(PUCHAR pszProcessName, BOOLEAN bRemove,BOOLEAN bMonitor) ;
static ULONG   Psi_AddProcessInfo(PUCHAR pszProcessName, BOOLEAN bMonitor) ;
static ULONG   Psi_DelProcessInfo(PUCHAR pszProcessName, BOOLEAN bMonitor) ;

VOID Ps_ProcessCallBack(
	__in HANDLE hParentId,
	__in HANDLE hProcessId,
	__in BOOLEAN bCreate
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	PEPROCESS EProcess ;
	UCHAR szProcessName[16] = {0} ;	

	UNREFERENCED_PARAMETER(bCreate) ;
	UNREFERENCED_PARAMETER(hParentId) ;

	try{

		if (!bCreate)
			_leave ;

		/**
		 * get process environment block(PEB)
		 *  The PsLookupProcessByProcessId routine accepts the process ID of a process and returns a 
		 *  referenced pointer to EPROCESS structure of the process.
		 */
		status = PsLookupProcessByProcessId(hProcessId, &EProcess) ;
		if (!NT_SUCCESS(status))
		{
			_leave ;
		}

		/**
		 * get process name
		 */
		//Ps_GetProcessName(szProcessName, EProcess) ;

		/**
		 * add process info in list, if exists, donnot insert into list again.
		 */
		//Psi_AddProcessInfo(szProcessName, FALSE) ;

	}
	finally{
		/**/
		//Todo some post work here
	}
}

/*
 * Monitoring Process ¡£
 */
extern KSPIN_LOCK	g_SpinLockForProcessRecognition;
WCHAR wszFilePathName[512];
CHAR szProcessName[64];
BOOLEAN Ps_IsCurrentProcessMonitored(WCHAR* pwszFilePathName, ULONG uLength, BOOLEAN* bIsSystemProcess, BOOLEAN* bIsPPTFile)
{
	KIRQL irql;
	int nIndex = 0;
	BOOLEAN bRet = FALSE ;
	BOOLEAN bExcelExt = FALSE ;
	WCHAR* pwszExt = NULL ;
	PLIST_ENTRY TmpListEntryPtr = NULL ;
	PiPROCESS_INFO psProcessInfo = NULL ;
	
	KeAcquireSpinLock(&g_SpinLockForProcessRecognition, &irql);
	
	try{
		// get current process name. 

		RtlZeroMemory(szProcessName,64);
		Ps_GetProcessName(szProcessName, NULL) ;
		

		// save file path name in local buffer

		RtlZeroMemory(wszFilePathName,1024);
		RtlCopyMemory(wszFilePathName, pwszFilePathName, uLength*sizeof(WCHAR)) ;

		// recognize process name and return to caller
		if (bIsSystemProcess != NULL)
		{
		    	if( ((strlen(szProcessName) == strlen("excel.exe")) && !_strnicmp(szProcessName, "excel.exe", strlen("excel.exe")))|| ((strlen(szProcessName) == strlen("et.exe")) && !_strnicmp(szProcessName, "et.exe", strlen("et.exe"))))
				{
					*bIsSystemProcess = EXCEL_PROCESS ;					
				}
		    	else if ((strlen(szProcessName) == strlen("powerpnt.exe")) && !_strnicmp(szProcessName, "powerpnt.exe", strlen("powerpnt.exe")))
				{
					*bIsSystemProcess = POWERPNT_PROCESS ;
				}else
				{
					*bIsSystemProcess = NORMAL_PROCESS ;
				}				
		}

		_wcslwr(wszFilePathName) ;	
		
		if (wcsstr(wszFilePathName, L"\\local settings\\temp"))//\Local Settings\Temp
		{
			bRet = TRUE ;
			__leave ;
		}
		
		// go to end of file path name, save pointer in pwszExt
		pwszExt = wszFilePathName + uLength - 1 ;

		// verify file attribute, if directory, return false
		if (pwszFilePathName[uLength-1] == L'\\')
		{//if directory, filter it
			bRet = FALSE ;
#ifdef DBG
			////DbgPrint("It is a dir:False\n");	
#endif
			__leave ;
		}

		// redirect to file extension name(including point)
		while (((pwszExt != wszFilePathName) && (*pwszExt != L'\\')) && ((*pwszExt) != L'.'))
		{//direct into file extension
			pwszExt -- ;
		}
		// verify this is a file without extension name
		if ((pwszExt == wszFilePathName) || (*pwszExt == L'\\'))
		{//no file extension exists in input filepath name, filter it.
			bRet = FALSE ;
			
			if((( pwszExt != wszFilePathName) && (*pwszExt == L'\\')) && !_strnicmp(szProcessName, "excel.exe", strlen("excel.exe")))
			{
				//DbgPrint("excel.exe and no ext file ~~~\n");
				bExcelExt = TRUE;
			}else
			{
				__leave ;
			}		
		}
	
		// verify tmp file
		if(!_strnicmp(szProcessName, "powerpnt.exe", strlen("powerpnt.exe")))
		if ((bIsPPTFile != NULL) && !_wcsnicmp(pwszExt, L".ppt", wcslen(L".ppt")))
		{
			*bIsPPTFile = TRUE ;
		}

		// verify tmp file
		if(!_strnicmp(szProcessName, "powerpnt.exe", strlen("powerpnt.exe")))
		if ((bIsPPTFile != NULL) && !_wcsnicmp(pwszExt, L".pptx", wcslen(L".pptx")))
		{
			*bIsPPTFile = TRUE ;
		}
		

		// verify tmp file
		if(!_strnicmp(szProcessName, "powerpnt.exe", strlen("powerpnt.exe")))
		if ((bIsPPTFile != NULL) && !_wcsnicmp(pwszExt, L".tmp", wcslen(L".tmp")))
		{
			*bIsPPTFile = TRUE ;
		}

		// compare current process name with process info in monitored list
		// if existing, match file extension name
		TmpListEntryPtr = g_ProcessListHead.Flink ;
		
		while(&g_ProcessListHead != TmpListEntryPtr)
		{
			psProcessInfo = CONTAINING_RECORD(TmpListEntryPtr, iPROCESS_INFO, ProcessList) ;

			if(!_strnicmp(psProcessInfo->szProcessName, szProcessName, strlen(szProcessName)))
			{
				nIndex = 0 ;

				if (!_strnicmp(psProcessInfo->szProcessName, "excel.exe", strlen("excel.exe")))
				{
					if (bExcelExt==TRUE)
					{
						bRet = TRUE;					
						__leave ;
					}
				}

				if (!_strnicmp(psProcessInfo->szProcessName, "et.exe", strlen("et.exe")))
				{
					if (bExcelExt==TRUE)
					{						
						bRet = TRUE;
						__leave ;
					}
				}
				
				while (TRUE)
				{
					// judge whether current file extension name is matched with monitored file type in list
					if (psProcessInfo->wsszRelatedExt[nIndex][0] == L'\0')
					{						
						bRet = FALSE ;
						break ;
					}
					else if ((wcslen(pwszExt) == wcslen(psProcessInfo->wsszRelatedExt[nIndex])) && !_wcsnicmp(pwszExt, psProcessInfo->wsszRelatedExt[nIndex], wcslen(pwszExt)))
					{
						bRet = TRUE;
						__leave ;
						
					}
					nIndex ++ ;
				}				
			}

			// move to next process info in list
			TmpListEntryPtr = TmpListEntryPtr->Flink ;
	}
		
		////DbgPrint("Nothing to compare ~ \n");
		
		bRet = FALSE ;
	}
	finally{
		/**/
		//Todo some post work here
	}
	KeReleaseSpinLock(&g_SpinLockForProcessRecognition, irql);
	return bRet ;
}


PVOID
Ps_GetAllProcessInfo(
   __out PVOID  pProcessInfo,
   __out PULONG puCount
   )
{
	KIRQL oldIrql ;
	PLIST_ENTRY TmpListEntryPtr = NULL ;
	PiPROCESS_INFO psProcessInfo = NULL ;
	PMSG_GET_ALL_PROCESS_INFO psGetAllProcInfo = (PMSG_GET_ALL_PROCESS_INFO)pProcessInfo ;

	try{

		*puCount = 0 ;
		KeAcquireSpinLock(&g_ProcessListLock, &oldIrql) ;
		TmpListEntryPtr = g_ProcessListHead.Flink ;
		while(&g_ProcessListHead != TmpListEntryPtr)
		{
			//when it is reply fot IOCTL_GET_PROCESS_COUNT
			if (NULL != psGetAllProcInfo)
			{//get all process info if needed
				psProcessInfo = CONTAINING_RECORD(TmpListEntryPtr, iPROCESS_INFO, ProcessList) ;
				RtlCopyMemory(psGetAllProcInfo->sProcInfo[*puCount].szProcessName, psProcessInfo->szProcessName, strlen(psProcessInfo->szProcessName)) ;
				psGetAllProcInfo->sProcInfo[*puCount].bMonitor = psProcessInfo->bMonitor ;
				psGetAllProcInfo->uCount ++ ;
			}

			(*puCount) ++ ; //get process count
			TmpListEntryPtr = TmpListEntryPtr->Flink ;
		}

	}
	finally{
		/**/
		//Todo some post work here
		KeReleaseSpinLock(&g_ProcessListLock, oldIrql) ;
	}

	return pProcessInfo ;
}


BOOLEAN
Ps_SetProcessInfo(
   __in PVOID InputBuffer
   )
{
	PMSG_SEND_SET_PROCESS_INFO psSendSetProcInfo = (PMSG_SEND_SET_PROCESS_INFO)InputBuffer ;
	try{
		if (NULL == psSendSetProcInfo)
			_leave ;
		if (IOCTL_SET_PROCESS_MONITOR == psSendSetProcInfo->sSendType.uSendType)
		{
			Psi_SetProcessMonitor(&psSendSetProcInfo->sProcInfo, FALSE) ;
		}
	}
	finally{
	}

	return TRUE ;
}


VOID Ps_AddProcessInfo(PVOID pAddProcInfo, PVOID pAddProcRes)
{
	PMSG_SEND_ADD_PROCESS_INFO psSendAddProcInfo = (PMSG_SEND_ADD_PROCESS_INFO)pAddProcInfo ;
	PMSG_GET_ADD_PROCESS_INFO psGetAddProcInfo = (PMSG_GET_ADD_PROCESS_INFO)pAddProcRes ;

	try{
		if ((NULL == psSendAddProcInfo) || (NULL == psGetAddProcInfo))
		{
			_leave ;
		}
		psGetAddProcInfo->uResult = Psi_AddProcessInfo(psSendAddProcInfo->sProcInfo.szProcessName, TRUE);//psSendAddProcInfo->sProcInfo.bMonitor) ;
	}
	finally{
	}
}

VOID Ps_DelProcessInfo(PVOID pDelProcInfo, PVOID pDelProcRes)
{
	PMSG_SEND_DEL_PROCESS_INFO psSendDelProcInfo = (PMSG_SEND_DEL_PROCESS_INFO)pDelProcInfo ;
	PMSG_GET_DEL_PROCESS_INFO psGetDelProcInfo = (PMSG_GET_DEL_PROCESS_INFO)pDelProcRes ;

	try{
		if ((NULL == psSendDelProcInfo) || (NULL == psGetDelProcInfo))
		{
			_leave ;
		}
		psGetDelProcInfo->uResult = Psi_DelProcessInfo(psSendDelProcInfo->sProcInfo.szProcessName, psSendDelProcInfo->sProcInfo.bMonitor) ;
	}
	finally{
	}
}


ULONG Psi_AddProcessInfo(PUCHAR pszProcessName, BOOLEAN bMonitor)
{
	ULONG uRes = MGAPI_RESULT_SUCCESS ;
	PiPROCESS_INFO psProcInfo = NULL ;
	BOOLEAN bRet ;

	try{
		if (NULL == pszProcessName)
		{
			uRes = MGAPI_RESULT_INTERNEL_ERROR ;
			_leave ;
		}

		/**
		* search for process name, if exists, donnot insert again
		*/
		bRet = Psi_SearchForSpecifiedProcessInList(pszProcessName, FALSE,bMonitor) ;
		if (bRet)
		{
			//DbgPrint("MGAPI_RESULT_ALREADY_EXIST pszProcessName %s\n",pszProcessName);
			uRes = MGAPI_RESULT_ALREADY_EXIST ;
			_leave ;
		}

		/**
		* allocate process info structure
		*/
		psProcInfo = ExAllocatePool(NonPagedPool, sizeof(iPROCESS_INFO)) ;
		if (NULL == psProcInfo)
		{
			//DbgPrint("******** MGAPI_RESULT_INTERNEL_ERROR\n");
			
			uRes = MGAPI_RESULT_INTERNEL_ERROR ;
			_leave ;
		}

		RtlZeroMemory(psProcInfo, sizeof(iPROCESS_INFO)) ;

		//DbgPrint("******** Prepare initialize .... \n");

		/**
		* initialize process info and insert it into global process list
		*/
		if (!_strnicmp(pszProcessName, "winword.exe", strlen("winword.exe")))
		{
			wcscpy(psProcInfo->wsszRelatedExt[0],  L".html") ;
			wcscpy(psProcInfo->wsszRelatedExt[1],  L".txt") ;
			wcscpy(psProcInfo->wsszRelatedExt[2],  L".mh_") ; //relative to .mht and .mhtml extension
			wcscpy(psProcInfo->wsszRelatedExt[3],  L".rtf") ;
			wcscpy(psProcInfo->wsszRelatedExt[4],  L".ht_") ; //relative to .htm and .html extension
			wcscpy(psProcInfo->wsszRelatedExt[5],  L".xml") ;
			wcscpy(psProcInfo->wsszRelatedExt[6],  L".mht") ;
			wcscpy(psProcInfo->wsszRelatedExt[7],  L".mhtml") ;
			wcscpy(psProcInfo->wsszRelatedExt[8],  L".htm") ;
			wcscpy(psProcInfo->wsszRelatedExt[9],  L".dot") ;
			wcscpy(psProcInfo->wsszRelatedExt[10], L".tmp") ;
			wcscpy(psProcInfo->wsszRelatedExt[11], L".docm") ;
			wcscpy(psProcInfo->wsszRelatedExt[12], L".docx") ;
			wcscpy(psProcInfo->wsszRelatedExt[13], L".doc") ;
		}
		else if (!_strnicmp(pszProcessName, "wps.exe", strlen("wps.exe")))
		{
			wcscpy(psProcInfo->wsszRelatedExt[0], L".wps") ;
			wcscpy(psProcInfo->wsszRelatedExt[1], L".mpt") ;	
			wcscpy(psProcInfo->wsszRelatedExt[2], L".doc") ;
			wcscpy(psProcInfo->wsszRelatedExt[3], L".dot") ;
			wcscpy(psProcInfo->wsszRelatedExt[4], L".txt") ;
			wcscpy(psProcInfo->wsszRelatedExt[5], L".docx") ;
			wcscpy(psProcInfo->wsszRelatedExt[6], L".tmp") ;
		}
		else if (!_strnicmp(pszProcessName, "excel.exe", strlen("excel.exe")))
		{
			wcscpy(psProcInfo->wsszRelatedExt[0],  L".xls") ;
			wcscpy(psProcInfo->wsszRelatedExt[1],  L".xml") ;
			wcscpy(psProcInfo->wsszRelatedExt[2],  L".mht") ;
			wcscpy(psProcInfo->wsszRelatedExt[3],  L".mhtml") ;
			wcscpy(psProcInfo->wsszRelatedExt[4],  L".htm") ;
			wcscpy(psProcInfo->wsszRelatedExt[5],  L".html") ;
			wcscpy(psProcInfo->wsszRelatedExt[6],  L".mh_") ; //relative to .mht extension
			wcscpy(psProcInfo->wsszRelatedExt[7],  L".ht_") ; //relative to .htm and .html extension
			wcscpy(psProcInfo->wsszRelatedExt[8],  L".xlt") ;
			wcscpy(psProcInfo->wsszRelatedExt[9],  L".txt") ;
			wcscpy(psProcInfo->wsszRelatedExt[10], L".tmp") ;
			wcscpy(psProcInfo->wsszRelatedExt[11], L".") ;
			wcscpy(psProcInfo->wsszRelatedExt[12], L".xlsx") ;
		}
		else if (!_strnicmp(pszProcessName, "et.exe", strlen("et.exe")))
		{
			wcscpy(psProcInfo->wsszRelatedExt[0], L".et") ;
			wcscpy(psProcInfo->wsszRelatedExt[1], L".ett") ;	
			wcscpy(psProcInfo->wsszRelatedExt[2], L".xls") ;
			wcscpy(psProcInfo->wsszRelatedExt[3], L".xlt") ;
			wcscpy(psProcInfo->wsszRelatedExt[4], L".txt") ;
			wcscpy(psProcInfo->wsszRelatedExt[5], L".csv") ;
			wcscpy(psProcInfo->wsszRelatedExt[6], L".dbf") ;
			wcscpy(psProcInfo->wsszRelatedExt[7], L".xlsx") ;

		}
		else if (!_strnicmp(pszProcessName, "powerpnt.exe", strlen("powerpnt.exe")))
		{
			wcscpy(psProcInfo->wsszRelatedExt[0],  L".ppt") ;
			wcscpy(psProcInfo->wsszRelatedExt[1],  L".tmp") ;
			wcscpy(psProcInfo->wsszRelatedExt[2],  L".rtf") ;
			wcscpy(psProcInfo->wsszRelatedExt[3],  L".pot") ;
			wcscpy(psProcInfo->wsszRelatedExt[4],  L".ppsm") ;
			wcscpy(psProcInfo->wsszRelatedExt[5],  L".mht") ;
			wcscpy(psProcInfo->wsszRelatedExt[6],  L".mhtml") ;
			wcscpy(psProcInfo->wsszRelatedExt[7],  L".htm") ;
			wcscpy(psProcInfo->wsszRelatedExt[8],  L".html") ;	
			wcscpy(psProcInfo->wsszRelatedExt[9],  L".pps") ;
			wcscpy(psProcInfo->wsszRelatedExt[10], L".ppa") ;
			wcscpy(psProcInfo->wsszRelatedExt[11], L".pptx") ;
			wcscpy(psProcInfo->wsszRelatedExt[12], L".pptm") ;
			wcscpy(psProcInfo->wsszRelatedExt[13], L".potx") ;
			wcscpy(psProcInfo->wsszRelatedExt[14], L".potm") ;
			wcscpy(psProcInfo->wsszRelatedExt[15], L".ppsx") ;
			wcscpy(psProcInfo->wsszRelatedExt[16], L".mh_") ; //relative to .mht and .mhtml extension
			wcscpy(psProcInfo->wsszRelatedExt[17], L".ht_") ; //relative to .htm and .html extension	
			wcscpy(psProcInfo->wsszRelatedExt[18], L".pptx") ;
		}		
		else if (!_strnicmp(pszProcessName, "wpp.exe", strlen("wpp.exe")))
		{
			wcscpy(psProcInfo->wsszRelatedExt[0],  L".ppt") ;
			wcscpy(psProcInfo->wsszRelatedExt[1],  L".tmp") ;
			wcscpy(psProcInfo->wsszRelatedExt[2],  L".rtf") ;
			wcscpy(psProcInfo->wsszRelatedExt[3],  L".pot") ;
			wcscpy(psProcInfo->wsszRelatedExt[4],  L".ppsm") ;
			wcscpy(psProcInfo->wsszRelatedExt[5],  L".mht") ;
			wcscpy(psProcInfo->wsszRelatedExt[6],  L".mhtml") ;
			wcscpy(psProcInfo->wsszRelatedExt[7],  L".htm") ;
			wcscpy(psProcInfo->wsszRelatedExt[8],  L".html") ;	
			wcscpy(psProcInfo->wsszRelatedExt[9],  L".pps") ;
			wcscpy(psProcInfo->wsszRelatedExt[10], L".ppa") ;
			wcscpy(psProcInfo->wsszRelatedExt[11], L".pptx") ;
			wcscpy(psProcInfo->wsszRelatedExt[12], L".pptm") ;
			wcscpy(psProcInfo->wsszRelatedExt[13], L".potx") ;
			wcscpy(psProcInfo->wsszRelatedExt[14], L".potm") ;
			wcscpy(psProcInfo->wsszRelatedExt[15], L".ppsx") ;
			wcscpy(psProcInfo->wsszRelatedExt[16], L".mh_") ; //relative to .mht and .mhtml extension
			wcscpy(psProcInfo->wsszRelatedExt[17], L".ht_") ; //relative to .htm and .html extension	
			wcscpy(psProcInfo->wsszRelatedExt[18], L".pptx") ;
			wcscpy(psProcInfo->wsszRelatedExt[19], L".dps") ;
			wcscpy(psProcInfo->wsszRelatedExt[20], L".dpt") ;
		}
		else if (!_strnicmp(pszProcessName, "System", strlen("System")))
		{
			wcscpy(psProcInfo->wsszRelatedExt[0],  L".txt") ;
			wcscpy(psProcInfo->wsszRelatedExt[1],  L".docx") ;
			wcscpy(psProcInfo->wsszRelatedExt[2],  L".doc") ;
			wcscpy(psProcInfo->wsszRelatedExt[3],  L".wps") ;
			wcscpy(psProcInfo->wsszRelatedExt[4],  L".dot") ;
			wcscpy(psProcInfo->wsszRelatedExt[5],  L".mpt") ;
			wcscpy(psProcInfo->wsszRelatedExt[6],  L".tmp") ;
			wcscpy(psProcInfo->wsszRelatedExt[7],  L".ppt") ;
			wcscpy(psProcInfo->wsszRelatedExt[8],  L".pptx") ;
			wcscpy(psProcInfo->wsszRelatedExt[9],  L".xls") ;
			wcscpy(psProcInfo->wsszRelatedExt[10], L".xlsx") ;
			wcscpy(psProcInfo->wsszRelatedExt[11], L".") ;
			

		}
		
		psProcInfo->bMonitor = TRUE;//bMonitor ;
		RtlCopyMemory(psProcInfo->szProcessName, pszProcessName, strlen(pszProcessName)) ;

		ExInterlockedInsertTailList(&g_ProcessListHead, &psProcInfo->ProcessList, &g_ProcessListLock) ;
	}
	finally{
	}

	return uRes ;
}


ULONG Psi_DelProcessInfo(PUCHAR pszProcessName, BOOLEAN bMonitor) 
{
	ULONG uRes = MGAPI_RESULT_SUCCESS ;
	BOOLEAN bRet ;

	try{
		if (NULL == pszProcessName)
		{
			uRes = MGDPI_RESULT_INTERNEL_ERROR ;
			_leave ;
		}

		/**
		* search for process name, if exists, donnot insert again
		*/
		bRet = Psi_SearchForSpecifiedProcessInList(pszProcessName, TRUE,bMonitor) ;
		if (!bRet)
		{
			uRes = MGDPI_RESULT_NOT_EXIST ;
			_leave ;
		}
	}
	finally{
	}

	return uRes ;
}


BOOLEAN
Ps_SetMonitored(
   __in PVOID InputBuffer
   )
{
	PMSG_SEND_SET_PROCESS_INFO psSendSetProcInfo = (PMSG_SEND_SET_PROCESS_INFO)InputBuffer ;
	try{
		if (NULL == psSendSetProcInfo)
			_leave ;
		if (IOCTL_SET_MONITOR == psSendSetProcInfo->sSendType.uSendType)
		{
			Psi_SetProcessMonitor(&psSendSetProcInfo->sProcInfo, TRUE) ;
		}
	}
	finally{
	}

	return TRUE ;
}


BOOLEAN
Ps_GetMonitorStatus(
   __out PVOID OutputBuffer
   )
{	
	KIRQL oldIrql ;
	PLIST_ENTRY TmpListEntryPtr = NULL ;
	PiPROCESS_INFO psProcessInfo = NULL ;
	PMSG_SEND_SET_PROCESS_INFO psSendSetProcInfo = (PMSG_SEND_SET_PROCESS_INFO)OutputBuffer ;
	
	try{
		if (NULL == psSendSetProcInfo)
			_leave ;
		KeAcquireSpinLock(&g_ProcessListLock, &oldIrql) ;

		
		TmpListEntryPtr = g_ProcessListHead.Flink ;
		
		while(&g_ProcessListHead != TmpListEntryPtr)
		{
			psProcessInfo = CONTAINING_RECORD(TmpListEntryPtr, iPROCESS_INFO, ProcessList) ;
			if (!_strnicmp(psProcessInfo->szProcessName, psSendSetProcInfo->sProcInfo.szProcessName, strlen(psSendSetProcInfo->sProcInfo.szProcessName)))
			{	
				psSendSetProcInfo->sProcInfo.bMonitor = psProcessInfo->bMonitor ;
				break ;
			}
	
			TmpListEntryPtr = TmpListEntryPtr->Flink ;
		}

		
		KeReleaseSpinLock(&g_ProcessListLock, oldIrql) ;
	}
	finally{
	}

	return TRUE ;
}

BOOLEAN Psi_SetProcessMonitor(PPROCESS_INFO psProcInfo, BOOLEAN bAll)
{
	BOOLEAN bRet = TRUE ;
	KIRQL oldIrql ;
	PLIST_ENTRY TmpListEntryPtr = NULL ;
	PiPROCESS_INFO psProcessInfo = NULL ;

	try{

		KeAcquireSpinLock(&g_ProcessListLock, &oldIrql) ;
		TmpListEntryPtr = g_ProcessListHead.Flink ;
		while(&g_ProcessListHead != TmpListEntryPtr)
		{
			psProcessInfo = CONTAINING_RECORD(TmpListEntryPtr, iPROCESS_INFO, ProcessList) ;

			if (!bAll)
			{
				if (!_strnicmp(psProcessInfo->szProcessName, psProcInfo->szProcessName, strlen(psProcInfo->szProcessName)))
				{
					psProcessInfo->bMonitor = psProcInfo->bMonitor ;
					break ;
				}
			}
			else
			{
				psProcessInfo->bMonitor = psProcInfo->bMonitor ;
			}

			TmpListEntryPtr = TmpListEntryPtr->Flink ;
		}

		if(!bAll)
			bRet = FALSE ;
	}
	finally{
		/**/
		//Todo some post work here
		KeReleaseSpinLock(&g_ProcessListLock, oldIrql) ;
	}

	return bRet ;
}


/*
 *find a process in the processlist according to the first parameter, and this item would be deleted if the second parameter
 *is true. If target process is found, it returns true without considering second parameter, else return false.
*/
static BOOLEAN Psi_SearchForSpecifiedProcessInList(PUCHAR pszProcessName, BOOLEAN bRemove,BOOLEAN bMonitor)
{
	BOOLEAN bRet = TRUE ;
	KIRQL oldIrql ;
	PLIST_ENTRY TmpListEntryPtr = NULL ;
	PiPROCESS_INFO psProcessInfo = NULL ;

	try{

		TmpListEntryPtr = g_ProcessListHead.Flink ;
		while(&g_ProcessListHead != TmpListEntryPtr)
		{
			psProcessInfo = CONTAINING_RECORD(TmpListEntryPtr, iPROCESS_INFO, ProcessList) ;

			if (!_strnicmp(psProcessInfo->szProcessName, pszProcessName, strlen(pszProcessName)))
			{
				psProcessInfo->bMonitor = bMonitor;
				//DbgPrint("******** psProcessInfo->bMonitor:%u  pszProcessName is %s\n",bRet,szProcessName);
				bRet = TRUE;
				if (bRemove)
				{
					KeAcquireSpinLock(&g_ProcessListLock, &oldIrql) ;
					RemoveEntryList(&psProcessInfo->ProcessList) ;
					KeReleaseSpinLock(&g_ProcessListLock, oldIrql) ;
					ExFreePool(psProcessInfo) ;
					psProcessInfo = NULL ;
				}
				__leave ;
			}

			TmpListEntryPtr = TmpListEntryPtr->Flink ;
		}

		bRet = FALSE ;
	}
	finally{
		/**/
		//Todo some post work here
	}

	return bRet ;
}

PCHAR Ps_GetProcessName(PCHAR pszProcessName, PEPROCESS pEProcess)
{
	PEPROCESS curproc = pEProcess;
	char *nameptr ;

	if (g_nProcessNameOffset)
	{
		if (!curproc)
		{
			curproc = PsGetCurrentProcess() ;
		}
		nameptr = (PCHAR)curproc + g_nProcessNameOffset ;
		strncpy(pszProcessName, nameptr, 15) ;
	}
	else
	{
		strcpy(pszProcessName, "???") ;
	}

	return pszProcessName ;
}

ULONG
Ps_GetProcessNameOffset(
	VOID
	)
{
	PEPROCESS curproc = NULL ;
	int i = 0 ;

	curproc = PsGetCurrentProcess() ;

	for (i=0; i<3*PAGE_SIZE; i++)
	{
		if (!strncmp("System", (PCHAR)curproc+i, strlen("System")))
		{
			return i ;
		}
	}

	return 0 ;
}
