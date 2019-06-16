in base.f
in io.f

: test 23 24 + ;

: loop-test dup 0 = if drop exit . 1 - tail ;

: case-test 2
	0 case exit
	1 case exit
	2 case . exit
	3 case exit
	4 case exit ;

test
loop-test
case-test

load ./test_lib.so

c_test

: &test & c_test call ;
&test

: string-test " test!" puts ;
string-test

: num-test 2345 putx 10 putc ;
num-test

: assert-test 3 2 = " Should be equal" assert ;
assert-test
