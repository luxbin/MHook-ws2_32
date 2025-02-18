#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include "injector.h"

BOOL InjectDll(char* processPath, char* dllPath)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL success = FALSE;

    // Create the process in suspended state
    if (!CreateProcessA(
        processPath,        // Application name
        NULL,              // Command line
        NULL,              // Process handle not inheritable
        NULL,              // Thread handle not inheritable
        FALSE,             // Set handle inheritance to FALSE
        CREATE_SUSPENDED,  // Create in suspended state
        NULL,             // Use parent's environment block
        NULL,             // Use parent's starting directory 
        &si,              // Pointer to STARTUPINFO structure
        &pi))             // Pointer to PROCESS_INFORMATION structure
    {
        printf("CreateProcess failed (%d)\n", GetLastError());
        return FALSE;
    }

    // Get handle to kernel32.dll
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32)
    {
        printf("Failed to get Kernel32 handle (%d)\n", GetLastError());
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return FALSE;
    }

    // Get address of LoadLibraryA
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
    if (!pLoadLibrary)
    {
        printf("Failed to get LoadLibraryA address (%d)\n", GetLastError());
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return FALSE;
    }

    // Allocate memory in target process for DLL path
    size_t dllPathLen = strlen(dllPath) + 1;
    LPVOID pRemoteDllPath = VirtualAllocEx(
        pi.hProcess,
        NULL,
        dllPathLen,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);

    if (!pRemoteDllPath)
    {
        printf("VirtualAllocEx failed (%d)\n", GetLastError());
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return FALSE;
    }

    // Write DLL path to target process memory
    if (!WriteProcessMemory(
        pi.hProcess,
        pRemoteDllPath,
        dllPath,
        dllPathLen,
        NULL))
    {
        printf("WriteProcessMemory failed (%d)\n", GetLastError());
        VirtualFreeEx(pi.hProcess, pRemoteDllPath, 0, MEM_RELEASE);
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return FALSE;
    }

    // Create remote thread to load DLL
    HANDLE hRemoteThread = CreateRemoteThread(
        pi.hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary,
        pRemoteDllPath,
        0,
        NULL);

    if (!hRemoteThread)
    {
        printf("CreateRemoteThread failed (%d)\n", GetLastError());
        VirtualFreeEx(pi.hProcess, pRemoteDllPath, 0, MEM_RELEASE);
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return FALSE;
    }

    // Wait for DLL injection to complete
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Get thread exit code
    DWORD exitCode = 0;
    if (GetExitCodeThread(hRemoteThread, &exitCode) && exitCode != 0)
    {
        success = TRUE;
    }

    // Resume the main thread
    ResumeThread(pi.hThread);

    // Cleanup
    VirtualFreeEx(pi.hProcess, pRemoteDllPath, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return success;
} 