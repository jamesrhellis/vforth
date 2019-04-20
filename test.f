in base.f
( Impliment the higher level forth in forth )
: find-word ( word-def word-start end  -- word-ref ) dup >r 1 w + @ ;
: find-word ( dict word-start end  -- word-ref ) @ find-word ;

: test 23 24 + ;

: loop-test dup 0 = if drop exit then . 1 - tail ;

test
loop-test
