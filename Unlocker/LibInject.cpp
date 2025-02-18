#include "pch.h"
#include "LibInject.h"

#include "..\Common\constants.h"

#include <string>

#include <psapi.h>
#include <TlHelp32.h>

#ifdef _PACK
#include <VMProtectSDK.h>
#endif

#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib")

BOOL Attached_DLL = FALSE;


//#pragma data_seg(".og")
BOOL	g_bPatched = FALSE;
int		g_nBrowserKind = -1;
//#pragma data_seg()
//#pragma comment(linker, "/SECTION:.og,RWS")


BOOL InjectDLL(DWORD procID, const char* dllPath)
{
	BOOL WPM = 0;

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);
	if (hProc == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WPM = WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);
	if (!WPM)
	{
		CloseHandle(hProc);
		return FALSE;
	}

	FARPROC funcAddress = (FARPROC)LoadLibraryA;
//#if defined _M_X64 
//	funcAddress = GetRemoteProcAddress(hProc, GetRemoteModuleHandle(hProc, "kernel32.dll"), "LoadLibraryA", 0, FALSE);
//#endif

	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)funcAddress, loc, 0, 0);

	/*HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);*/
	if (!hThread)
	{
		VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
		CloseHandle(hProc);
		return FALSE;
	}
	CloseHandle(hProc);
	VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
	CloseHandle(hThread);
	return TRUE;
}

DWORD GetProcId(const wchar_t* pn)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pE;
		pE.dwSize = sizeof(pE);

		if (Process32First(hSnap, &pE))
		{
			if (!pE.th32ProcessID)
				Process32Next(hSnap, &pE);
			do
			{
				if (!_wcsicmp(pE.szExeFile, pn))
				{
					procId = pE.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pE));
		}
	}
	CloseHandle(hSnap);
	return procId;
}

void SleepUsingWaitForSingleObject(DWORD milliseconds) {
	// Create a dummy event object
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (hEvent) {
		// Wait for the specified time
		WaitForSingleObject(hEvent, milliseconds);

		// Close the event handle
		CloseHandle(hEvent);
	}
	else {
		// If event creation fails, fall back to Sleep
		Sleep(milliseconds);
	}
}

DWORD WINAPI MonitorProcessThread(LPVOID lpParam) 
{

	char szDllPath[MAX_PATH];

	GetCurrentDirectoryA(MAX_PATH, szDllPath);

	std::string dllPath = szDllPath;

	std::string dllName = ENCRYPTED_STRING("\\msvcpcr70.dll");

	dllPath += dllName;

	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	while (TRUE && !Attached_DLL)
	{
		int pid = GetProcId(LOCKDOWN_PROCESS_NAME);
		if (pid != NULL) {

			//OutputDebugStringA("=================== .exe");
			InjectDLL(pid, dllPath.c_str());
		}
		
		
		pid = GetProcId(LOCKDOWN_PROCESS_NAME_1);
		if (pid != NULL) {

			//OutputDebugStringA("=================== LockDownBrowserOEM.exe");
			InjectDLL(pid, dllPath.c_str());
		}

		pid = GetProcId(LOCKDOWN_PROCESS_NAME_2);
		if (pid != NULL) {

			//OutputDebugStringA("=================== LockDownBrowserLabOEM.exe");
			InjectDLL(pid, dllPath.c_str());
		}
		
		/*
		pid = GetProcId(L"Guardian Browser.exe"); //ProctorU
		if (pid != NULL) {

			InjectDLL(pid, dllPath.c_str());
		}
		*/


		//Sleep(50);
		WaitForSingleObject(hEvent, 50);
	}

	CloseHandle(hEvent);

	return 0;
}
