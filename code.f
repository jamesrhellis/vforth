: imm dict @ 3 w + dup @ 1 or swap ! ;
: inline dict @ 3 w + dup @ 2 or swap ! ;

: buffer-pos buffer ; inline
: buffer-items buffer 1 w + ;

: buffer-push buffer-items buffer-pos @ + c! buffer-pos @ 1 + buffer-pos ! ;
: tail i-b buffer-push
	buffer-pos @ 0 swap - 
	dup buffer-push 8 rshift dup buffer-push 8 rshift buffer-push; imm
: branch-pad 0 buffer-push 0 buffer-push 0 buffer-push ; 
: branch-write 2dup c! 2dup 1 + c! 2 + c! ;
: if i-bz buffer-push buffer-pos @ branch-pad ; imm
: then dup buffer-pos @ swap - swap buffer-items + branch-write ; imm

: test 23 24 + ;

: loop-test dup 0 = if drop exit then tail ;

test
loop-test
