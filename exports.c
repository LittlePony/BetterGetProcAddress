#include<stdio.h>
#include<windows.h>
#include"exports.h"
#include"structs.h"

DLL_EXPORT dll_export = { 0 };


PDLL_EXPORT get_export() {
	PPEB peb = (PPEB)__readgsqword(0x60);
	PLIST_ENTRY head = &peb->LoaderData->InMemoryOrderModuleList;
	PLIST_ENTRY current = head;

	PLDR_DATA_TABLE_ENTRY dll = NULL;
	while ((current = current->Flink) != head)
	{
		dll = (PLDR_DATA_TABLE_ENTRY)((PCHAR)current - sizeof(LIST_ENTRY));
		if (!wcscmp(dll->BaseDllName.Buffer, NTDLL))
			break;
	}
	if (dll == NULL)
		return NULL;

	PVOID dll_base = dll->DllBase;
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)dll_base;
	PIMAGE_FILE_HEADER file_header = (PIMAGE_FILE_HEADER)((PCHAR)dll_base + dos_header->e_lfanew + sizeof(DWORD));
	PIMAGE_OPTIONAL_HEADER optional_header = (PIMAGE_OPTIONAL_HEADER)(file_header + 1);
	ULONG export_size = optional_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	ULONG export_rva = optional_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	if(!export_size)
		return NULL;

	PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)dll_base + export_rva);

	dll_export.DllBase = dll_base;
	dll_export.AddressOfNames = (PDWORD)((PCHAR)dll_base + exports->AddressOfNames);
	dll_export.AddressOfFunctions = (PDWORD)((PCHAR)dll_base + exports->AddressOfFunctions);
	dll_export.AddressOfNameOrdinals = (PWORD)((PCHAR)dll_base + exports->AddressOfNameOrdinals);
	dll_export.NumberOfNames = exports->NumberOfNames;

	return &dll_export;
}

DWORD get_syscall(PDLL_EXPORT dll, PCHAR func) {
	DWORD middle;
	DWORD left = 0;
	DWORD right = dll->NumberOfNames;
	int result;

	while (right != left) {
		middle = left + ((right - left) >> 1);
		result = strcmp((PCHAR)dll->DllBase + dll->AddressOfNames[middle], func);

		if (!result) {
			PVOID func_address = (PBYTE)dll->DllBase + dll->AddressOfFunctions[dll->AddressOfNameOrdinals[middle]];
			BYTE low = *((PBYTE)func_address + 4);
			BYTE high = *((PBYTE)func_address + 5);
			DWORD sc_num = (high << 8) | low;

			return sc_num;
		}
		else if (result < 0)
			left = middle;
		else
			right = middle;
	}
	return NULL;
}
