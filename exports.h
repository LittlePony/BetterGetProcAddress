#pragma once

#define NTDLL L"ntdll.dll"

typedef struct _DLL_EXPORT {
	PVOID DllBase;
	PDWORD AddressOfNames;
	PDWORD AddressOfFunctions;
	PWORD AddressOfNameOrdinals;
	DWORD NumberOfNames;
} DLL_EXPORT, *PDLL_EXPORT;

PDLL_EXPORT get_export(void);

DWORD get_syscall(PDLL_EXPORT dll, PCHAR func);
