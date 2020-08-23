#include "types.h"
#include <stdio.h>

void test_call(f_state *fs) {
	printf("HI\n");
}

void lib_init(syscall_register add, literal_register ladd) {
	add("c_test", test_call);
}
