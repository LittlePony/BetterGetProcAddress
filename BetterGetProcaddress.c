#include<stdio.h>
#include<windows.h>
#include"exports.h"
#include"structs.h"


// https://hfiref0x.github.io/syscalls.html

int main(){
	char* func1 = "NtCompressKey";
	int sc_num1 = (int)get_syscall(func1);

	if(!sc_num1){
		fprintf(stderr, "%s doesn't exist in ntdll\n", func1);
	}
	else
		printf("%s is %d\n", func1, sc_num1);

	char* func2 = "NtFlushWriteBuffer";
	int sc_num2 = (int)get_syscall(func2);

	if (!sc_num2) {
		fprintf(stderr, "%s doesn't exist in ntdll\n", func2);
	}
	else
		printf("%s is %d\n", func2, sc_num2);

	return 0;
}
