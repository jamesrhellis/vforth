in base.f
: test 23 24 + ;

: loop-test dup 0 = if drop exit then . 1 - tail ;

: case-test 2
	0 case exit then
	1 case exit then
	2 case . exit then
	3 case exit then
	4 case exit then ;

test
loop-test
case-test
