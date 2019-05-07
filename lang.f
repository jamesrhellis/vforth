in base.f
( This is a compiler for a higher level concatenative language to the
	virtual stack machine - 320ne lang )
( The higher level language enforces safety by:
	- Ensuring a limit on the maximum stack height
	- Static analysis on varible use to ensure no memory issues
	- Automated freeing of resources ?
	- Bounds checking on bounded pointers to avoid overflow
)
( Syntax/Semantics :
	- Static rules for ensuring memory safety:
		- Ownership may only be changed if no other references 
			are on the stack
		- References may only be created to global objects
		- References cannot be stored
	- Pointer validity is exposed as a language primative: ?
		this takes the meaning ' not-valid if drop /code/ then '
		e.g. : eg array @ 3 + ? 0 exit @ ;
		: eg2 optional-owner @@ ? error exit no-error ;
	- Language enforced optionals - using the above ? syntax
)

( Dictionary words )

( word format )
( many attributes are limited to small finite numbers: )
( 8 arguements + return values, 32 owners, 
	4 ownerships claimed from the stack ) 

: next ;
: next> next @ ;
: >next next ! ;
: name 1 w + ;
: name> name @ ;
: >name name ! ;
: args 3 w + ;	( Types of arguements - list )
: ret 3 w + 1 + ;	( Types of return items )
: rel-claim 3 w + 2 + ;	( Ownerships from stack claimed 
			  - 8 bit bitmap )
: types 4 w + ; ( Combined storage of return and call types )
: abs-claim 12 w + ;	( Global ownerships claimed - bitset )
: height 14 w + ;	( Max height increase of stacks 
			  - 8 bit list )
			( 0 - main , 1 - ret , 
			  2/3 reserved for fp / vec ) 
: height-main 14 w + ; 
: height-ret 14 w + 1 ;
: code 15 w + ; 	( Code - bounded pointer )

0 var dict !
: find ( word? name -- word? ) 0 case drop 0 exit
	dup >r name> string-eq if 2drop r> exit
	drop r> next> tail ;

: store ( dict word -- ) dup >r @ over next ! r> ! ;

( Type tracking stack words )

( stack format )
: top ;
: items ;

: ts-pop ( stack -- top ) dup top dup @ dup >r 1 - swap !
	items r> w + @ ;
: ts-push ( stack top -- ) dup top dup @ 1 + dup >r swap !
	items r> w + ! ;
: ts-swap ( stack ) dup ts-pop over ts-pop >r 
	over ts-push r> swap ts-push ;
: ts-dup ( stack ) dup items over top @ w + @ swap ts-push ;
: ts-over ( stack ) dup top @ 1 - w over items @ swap ts-push ;
: ts-pick ( n stack -- item ) over top swap - w swap items + @ ;
: ts-pick-ref ( n stack -- ref ) over top swap - w swap items + ;
: items 1 w + ;

( Type manipulation words )
( type format :
	0-10 type
	11 optional? ; does this type need checking for validity
	12 bounded? ; is this a ref / own to an array
	13-14 ref-type : 0 -> not a ref, 1 -> ref to stack item,
		2 -> ref to global item, 3 -> ownership
	15-20 ref? ; what object is this a ref of / 0 -> own > ref
	21 mark	; used for marking value in compiler
)
	
0 invert 10 lshift invert con type-mask
: type type-mask and ;
: >type type-mask invert and or ;

1 11 lshift con optional-mask
: optional? optional-mask and 0 != ;
: >optional optional-mask invert and swap optional-mask and or ;

1 12 lshift con bounded-mask
: bounded? bounded-mask and 0 != ;
: >bounded ( to own -- to ) bounded-mask invert and swap bounded-mask and or ;

0 invert 2 lshift invert con ref-type-mask
2 con ref-abs
3 con ref-rel
: ref-type 13 rshift ref-type-mask and ;
: >ref-type ( to type -- to ) ref-type-mask 13 lshift invert and swap
	ref-type-mask and 13 lshift or ;

0 invert 5 lshift invert con ref-mask
: ref 15 rshift ref-mask and ;
: >ref ( to own -- to ) ref-mask 15 lshift invert and swap
	ref-mask and 15 lshift or ;

( Type tracking state )

( Stack used for tracking types during interpretation )
16 con stack-size
stack-size 1 + w con stack-alloc-size
: alloc-stack stack-size 1 + w alloc ;
alloc-stack con iter-main-stack
alloc-stack con iter-ret-stack
( Stack used for compiling )
alloc-stack con comp-main-stack
alloc-stack con comp-ret-stack
( Saved return state for if statements )
alloc-stack con if-main-stack
alloc-stack con if-ret-stack

( Type checking )

: norm-ref ( type -- normalised-type ) 0 swap >ref dup ref-type
	ref-rel case ref-abs swap >ref-type exit drop ;
: type-neq ( type type -- neq ) norm-ref swap norm-ref != ;
: check-types ( n list list -- type-error? )
	0 case 2drop false exit 1 - >r
	dup 1 w + >r @ swap
	dup 1 w + >r @
	type-neq if r> r> r> 3drop true exit
	r> r> r> tail ;

( Ownership checking )

: set-bit ( bit -- bitmap ) 1 swap lshift ;
: check-add-ownership ( item add-bit? abs-bitmap rel-bitmap -- conflict? abs-bitmap rel-bitmap )
	swap >r dup ref set-bit swap ref-type
	ref-abs case over or r>
		if swap then over eor 0 != exit
	ref-rel case swap rot over or r>
		if swap then over eor 0 != >r swap r> exit
	2drop 0 ;
: check-list-ownership ( n arg-ref-bitmap list abs-bitmap rel-bitmap -- ownership-error? )
	0 case 4drop false exit >r
	dup 1 rshift >r 1 and 0 != ( check arg-ref-bitmap )
	swap dup 1 w - >r @	( get next item )
	check-add-ownership if r> r> r> 5drop true exit
	r> r> r> tail ;

( Interpreter code )

: own-check ( word -- error? ) 0 swap ( rel-bitmap )
	dup abs-claim @ ( abs-bitmap )
	swap rel-claim c@ ( arg-ref-bitmap )
	iter-main-stack 1 ts-pick-ref swap ( list )
	iter-main-stack top ( n )
	check-list-ownership ;

: type-check ( word -- error? ) 
	iter-main-stack over args ts-pick-ref 
	swap dup types swap args check-types ;

: height-check ( word -- error? )
	dup height-main iter-main-stack top + 
	stack-size > if true exit
	height-ret iter-ret-stack top + 
	stack-size > if true exit
	false ;

: call-word ( word ) code call ;

: interpreter-sketch ( )
	next-word 0 case 2drop exit
	dict find 0 case ( FIXME ERROR ) exit
	dup height-check if ( FIXME ERROR ) exit
	dup type-check if ( FIXME ERROR ) exit
	dup own-check if ( FIXME ERROR ) exit
	call-word tail ;
