: imm dict @ 3 w + dup @ 1 or swap ! ;
: inline dict @ 3 w + dup @ 2 or swap ! ;

: buffer-push buffer-items buffer-pos @ + c! buffer-pos @ 1 + buffer-pos ! ;
: buffer-push24 
	dup buffer-push 8 rshift dup buffer-push 8 rshift buffer-push ;
: tail i-b buffer-push
	buffer-pos @ 0 swap - buffer-push24 ; imm

: branch-pad 0 buffer-push 0 buffer-push 0 buffer-push ; 
: if i-bz buffer-push buffer-pos @ branch-pad ; imm
: then buffer-pos @ dup >r over - swap
	buffer-pos ! buffer-push24 
	r> buffer-pos ! ; imm

: is-space dup 13 = 
	over 10 = or 
	over 9 = or 
	swap 32 = or ;
: skip-space dup c@ is-space if 1 + tail then ;
: skip-non-space dup c@ is-space invert if 1 + tail then ;
: next-word skip-space dup skip-non-space ;
: next-word in-file @ next-word dup 
	c@ 0 = if drop exit then 
	0 over c! 1 + dup in-file ! swap ;
: word next-word ; imm

: head-neq dup c! >r 2swap dup c! r> != ;
: string-eq 
	head-neq if 2drop 2drop 0 exit then
	1 + 2dup < if 2drop 2drop 1 exit then
	2swap 1 + tail ;
: string-eq 2over - >r 2dup - r> != if 0 exit then 
	2over 2over string-eq ;
	

: ( word ) next-word string-eq if exit then tail ; imm

( Now we can have comments )

: test 23 24 + ;

: loop-test dup 0 = if drop drop exit then drop 1 -  ;
