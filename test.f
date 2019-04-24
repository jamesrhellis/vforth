in base.f
: test 23 24 + ;

: loop-test dup 0 = if drop exit then . 1 - tail ;

test
loop-test
