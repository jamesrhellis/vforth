: imm dict @ 3 w + dup @ 1 or swap ! ;
: inline dict @ 3 w + dup @ 2 or swap ! ;

: <= > invert ; inline
: >= < invert ; inline

: buffer-push buffer-items buffer-pos @ + c! buffer-pos @ 1 + buffer-pos ! ;
: buffer-push24 
	dup buffer-push 8 rshift dup buffer-push 8 rshift buffer-push ;
: tail I_B buffer-push
	buffer-pos @ 0 swap - buffer-push24 ; imm

: branch-pad 0 buffer-push24 ; 
: (if) I_BZ buffer-push buffer-pos @ branch-pad ; inline
: if (if) ; imm
: case I_OVER buffer-push I_EQ buffer-push
	(if) I_DROP buffer-push ; imm
: (then) buffer-pos @ dup >r over - swap
	buffer-pos ! buffer-push24 
	r> buffer-pos ! ; inline
: then (then) ; imm
: exit I_RET buffer-push (then) ; imm

: is-space dup 13 = 
	over 10 = or 
	over 9 = or 
	swap 32 = or ;
: is-not-space dup is-space swap 0 = or invert ;

: skip-space dup c@ is-space if 1 + tail then ;
: skip-non-space dup c@ is-not-space if 1 + tail then ;

: next-word skip-space dup skip-non-space ;
: next-word in-file @ next-word dup
	c@ 0 = if dup in-file ! 1 + swap exit 
	0 over c! 1 + dup in-file ! swap ;

: buffer-pushn dup 0 = if drop drop exit
	>r dup buffer-push 8 rshift 
	r> 1 - tail ;
: buffer-pushw 1 w buffer-pushn ;
: memcpy 2swap dup c@ >r 2swap r> over c!
	1 + 2dup <= if 2drop 2drop exit 
	2swap 1 + 2dup <= if 2drop 2drop exit 2swap tail ;

: string-dup 2dup - alloc 2over 2over memcpy 2swap 2drop ;
: word next-word string-dup swap
	I_IMMW buffer-push buffer-pushw
	I_IMMW buffer-push buffer-pushw ; imm

: head-neq dup c@ >r 2swap dup c@ r> != ;
: string-eq
	head-neq if 2drop 2drop 0 exit
	1 + 2dup <= if 2drop 2drop 1 exit
	2swap 1 + tail ;
: string-eq 2over - >r 2dup - r> != if 2drop 2drop 0 exit 
	 string-eq ;
	

: ( word ) next-word string-eq if exit tail ; imm

( Now we can have comments )

( Alloc wrappers )
: valloc ( size -- end start ) alloc ; inline
: alloc ( size -- start ) alloc swap drop ; inline
: vfree ( end start -- ) free drop ; inline

( Hacky call until I decide how to expose the interpreter to forth )
: call >r ;

( Variable Allocation space )
: var-buffer var-space ;
: var-pos var-space 1 w + ;

: var-buffer-alloc 128 w alloc var-buffer @ over ! 
	var-buffer ! 
	1 w var-pos ! ;
var-buffer-alloc

: var-alloc var-pos @ dup 128 w >= if var-buffer-alloc drop tail then
	dup 1 w + var-pos ! var-buffer @ + ;

: n-write ( value dest n ) dup 0 = if drop drop drop exit
	1 - >r over over c! swap 8 rshift swap 1 + r> tail ;
: immw-write ( dest value -- ) I_IMMW over c!
	1 + dup >r 1 w n-write I_RET r> 1 w + c! ;

: var-word ( value name -- ) 7 5 w + alloc
	swap over 1 w + ! ( name )
	1 1 w + over 2 w + ! ( len )
	swap over 4 w + immw-write 
	dict @ over ! dict ! inline ;

: var var-alloc dup next-word string-dup swap drop var-word ;
: con next-word string-dup swap drop var-word ;

0 con false
false invert con true

( Add wrapper for & to inline found word address )
: & & I_IMMW buffer-push buffer-pushw ; imm

( String parsing )
: skip-to-" dup c@ 
	0 case exit
	34 case exit
	drop 1 + tail ;
: " in-file @ dup skip-to-" dup dup c@ 
	0 != if 1 + then in-file !
	0 over c!
	I_IMMW buffer-push buffer-pushw 
	I_IMMW buffer-push buffer-pushw ; imm

