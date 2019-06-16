#include "types.h"
#include <stdio.h>

void test_call(STATE) {
	printf("HI\n");
}

void lib_init(syscall_register add) {
	add("c_test", test_call);
}
