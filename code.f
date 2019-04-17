: imm dict @ 3 w + dup @ 1 or swap ! ;
: inline dict @ 3 w + dup @ 2 or swap ! ;

: buffer-push buffer-items buffer-pos @ + c! buffer-pos @ 1 + buffer-pos ! ;
: buffer-push24 
	dup buffer-push 8 rshift dup buffer-push 8 rshift buffer-push ;
: tail i-b buffer-push
	buffer-pos @ 0 swap - buffer-push24 ; imm

: branch-pad 0 buffer-push 0 buffer-push 0 buffer-push ; 
: if i-bz buffer-push buffer-pos @ branch-pad ; imm
: then buffer-pos @ dup >r over - swap r> drop
	buffer-pos ! buffer-push24 
	r> buffer-pos ! ; imm

: test 23 24 + ;

: loop-test dup 0 = if drop drop exit then drop 1 -  ;
