#include<stdio.h>
#include<windows.h>
#include<winternl.h>
#include"exports.h"

DWORD get_syscall(PCHAR func){
	PPEB p = (PPEB)__readgsqword(0x60);
	PLIST_ENTRY main_module = &p -> Ldr -> InMemoryOrderModuleList;
	PLIST_ENTRY module = main_module -> Flink -> Flink; // skip to ntdll.
	PLDR_DATA_TABLE_ENTRY dll = (PLDR_DATA_TABLE_ENTRY)((PCHAR)module - sizeof(LIST_ENTRY));

	return get_export(dll -> DllBase, func);
}

DWORD get_export(PVOID base, PCHAR func){
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
	PIMAGE_FILE_HEADER head = (PIMAGE_FILE_HEADER)((PCHAR)base + dos -> e_lfanew + sizeof(DWORD));
	PIMAGE_OPTIONAL_HEADER opt_head = (PIMAGE_OPTIONAL_HEADER)(head + 1);
    ULONG export_size =  opt_head -> DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	ULONG export_rva = opt_head -> DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	if(!export_size)
		return NULL;

	PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)base + export_rva);

	PDWORD names_rva = (PDWORD)((PCHAR)base + exports -> AddressOfNames);
	PDWORD functions_rva = (PDWORD)((PCHAR)base + exports -> AddressOfFunctions);
	PWORD ordinals_rva = (PWORD)((PCHAR)base + exports -> AddressOfNameOrdinals);

    DWORD middle; // old_middle = 0;
	DWORD left = 0;
	DWORD right = exports -> NumberOfNames;

	while(right != left){
        middle = left + ((right - left) >> 1);
        int result = strcmp((PCHAR)base + names_rva[middle], func);

		if (!result) {
			PVOID func_address = (PBYTE)base + functions_rva[ordinals_rva[middle]];
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
