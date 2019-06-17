all: forth libs tests 

forth: csrc/*
	cc -std=c99 -ldl csrc/interpreter.c -o forth

libs: csrc/types.h csrc/test_lib.c  
	cc -std=c99 -fPIC -shared csrc/test_lib.c -o test_lib.so
	
tests: forth
	./forth test.f

