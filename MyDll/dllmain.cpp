// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "HookUtils.h"

#include "mhook-lib/mhook.h"

#include <string>

#include <tlhelp32.h>

#include "..\Common\constants.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")


BOOL InstallHook(HMODULE hModule);

HINSTANCE hInstance = NULL;

// Typedefs and Function Pointers
typedef int (WINAPI* _send)(SOCKET, const char*, int, int);
_send TrueSend = nullptr;

typedef int (WINAPI* _sendto)(SOCKET, const char*, int, int, const struct sockaddr*, int);
_sendto TrueSendTo = nullptr;

typedef int (WINAPI* _recv)(SOCKET, char*, int, int);
_recv TrueRecv = nullptr;

typedef int (WINAPI* _recvfrom)(SOCKET, char*, int, int, struct sockaddr*, int*);
_recvfrom TrueRecvFrom = nullptr;

typedef int (WINAPI* _WSASend)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
_WSASend TrueWSASend = nullptr;

typedef int (WINAPI* _WSARecv)(SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
_WSARecv TrueWSARecv = nullptr;

// Hooked Functions

int WINAPI HookSend(SOCKET s, const char* buf, int len, int flags) {
	printf("[Hook] send() called. Data: %.*s\n", len, buf);
	return TrueSend(s, buf, len, flags);
}

int WINAPI HookSendTo(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen) {
	printf("[Hook] sendto() called. Data: %.*s\n", len, buf);
	return TrueSendTo(s, buf, len, flags, to, tolen);
}

int WINAPI HookRecv(SOCKET s, char* buf, int len, int flags) {
	int result = TrueRecv(s, buf, len, flags);
	if (result > 0) {
		printf("[Hook] recv() called. Received Data: %.*s\n", result, buf);
	}
	return result;
}

int WINAPI HookRecvFrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen) {
	int result = TrueRecvFrom(s, buf, len, flags, from, fromlen);
	if (result > 0) {
		printf("[Hook] recvfrom() called. Received Data: %.*s\n", result, buf);
	}
	return result;
}

int WINAPI HookWSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	printf("[Hook] WSASend() called. Data: %.*s\n", lpBuffers[0].len, lpBuffers[0].buf);
	return TrueWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

int WINAPI HookWSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	int result = TrueWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
	if (result == 0 && *lpNumberOfBytesRecvd > 0) {
		printf("[Hook] WSARecv() called. Received Data: %.*s\n", *lpNumberOfBytesRecvd, lpBuffers[0].buf);
	}
	return result;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		hInstance = (HINSTANCE) hModule;
		return InstallHook(hModule);
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

BOOL InstallHook(HMODULE hModule)
{

	//WSADATA wsa;
	//WSAStartup(MAKEWORD(2, 2), &wsa);  // I am not sure if this part is needed or not.

	char szFileName[MAX_PATH];


	GetModuleFileNameA(GetModuleHandleA(NULL), szFileName, MAX_PATH);

	std::string target1 = "Target1.exe";
	std::string target2 = "Target2.exe";
	std::string target3 = "Target3.exe";


	if (!strstr(szFileName, target1.c_str()) &&
		!strstr(szFileName, target2.c_str()) &&
		!strstr(szFileName, target3.c_str())) {

		return FALSE;
	}

	HMODULE hWs2_32 = GetModuleHandleA("ws2_32.dll");

	// Get addresses of original functions
	TrueSend = (_send)GetProcAddress(hWs2_32, "send");
	TrueSendTo = (_sendto)GetProcAddress(hWs2_32, "sendto");
	TrueRecv = (_recv)GetProcAddress(hWs2_32, "recv");
	TrueRecvFrom = (_recvfrom)GetProcAddress(hWs2_32, "recvfrom");
	TrueWSASend = (_WSASend)GetProcAddress(hWs2_32, "WSASend");
	TrueWSARecv = (_WSARecv)GetProcAddress(hWs2_32, "WSARecv");

	// Set hooks using Mhook
	if (TrueSend) Mhook_SetHook((PVOID*)&TrueSend, (PVOID)HookSend);
	if (TrueSendTo) Mhook_SetHook((PVOID*)&TrueSendTo, (PVOID)HookSendTo);
	if (TrueRecv) Mhook_SetHook((PVOID*)&TrueRecv, (PVOID)HookRecv);
	if (TrueRecvFrom) Mhook_SetHook((PVOID*)&TrueRecvFrom, (PVOID)HookRecvFrom);
	if (TrueWSASend) Mhook_SetHook((PVOID*)&TrueWSASend, (PVOID)HookWSASend);
	if (TrueWSARecv) Mhook_SetHook((PVOID*)&TrueWSARecv, (PVOID)HookWSARecv);

	return TRUE;
}


