#pragma once

class CHookUtils
{
public:
	CHookUtils(void);
	~CHookUtils(void);
};

#define DWORDV *(DWORD*)

BOOL PatchJmpCode(DWORD dwHookAddr, DWORD dwHookFunc, void* pOrignBytes);