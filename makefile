all: forth libs tests 

forth: interpreter.c forth.c forth_modules.c types.h
	cc -std=c99 -ldl interpreter.c -o forth

libs: types.h test_lib.c  
	cc -std=c99 -fPIC -shared test_lib.c -o test_lib.so
	
tests: forth
	./forth test.f

