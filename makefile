all: forth libs tests 

forth: interpreter.c forth.c types.h
	gcc -ldl interpreter.c -o forth

libs: types.h test_lib.c  
	gcc -fPIC -shared test_lib.c -o test_lib.so

test:
	tcc -run interpreter.c test.f
	
tests: forth
	./forth test.f

