#include "StdAfx.h"
#include "HookUtils.h"

CHookUtils::CHookUtils(void)
{
}

CHookUtils::~CHookUtils(void)
{
}

BOOL PatchJmpCode(DWORD dwHookAddr, DWORD dwHookFunc, void* pOrignBytes)
{
	DWORD oldProtect;
	BOOL success = VirtualProtect((LPVOID)dwHookAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

	if (!success) {
		//OutputDebugStringA("Error VirtualProtect!");
		return FALSE;
	}

	BYTE jmpCodes[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

	DWORD relAddr = dwHookFunc - (dwHookAddr + sizeof(jmpCodes));
	memcpy(jmpCodes + 1, &relAddr, 4);
	memcpy((LPVOID)dwHookAddr, jmpCodes, sizeof(jmpCodes));

	VirtualProtect((LPVOID)dwHookAddr, 5, oldProtect, NULL);

	return TRUE;
}