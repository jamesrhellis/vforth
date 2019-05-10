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
: type-check-code 15 w + ; ( Code to determine type checking )
: code 16 w + ; 	( Code - bounded pointer )

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
	15-20 ref ; what object is this a ref of / 0 -> own > ref
	21 owner? ; is this value an owner - or in the 
			case of a ref, a ref to the owner
	
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
1 con ref-rel
2 con ref-abs
: ref-type 13 rshift ref-type-mask and ;
: >ref-type ( to type -- to ) ref-type-mask 13 lshift invert and swap
	ref-type-mask and 13 lshift or ;

0 invert 5 lshift invert con ref-mask
: ref 15 rshift ref-mask and ;
: >ref ( to own -- to ) ref-mask 15 lshift invert and swap
	ref-mask and 15 lshift or ;

1 21 lshift con owner-mask
: owner? owner-mask and 0 != ;
: >owner ( to own -- to ) owner-mask invert and swap owner-mask and or ;

: print-type ( type ) dup type putx 124 putc
	dup optional? putx 124 putc
	dup bounded? putx 124 putc
	dup ref-type putx 124 putc
	dup ref putx ;

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

( Basic words )
( Basic words need to be generic, and may produce different
  code depending on the width of types, so need custom
  type handling )
: type-width ( type -- width ) bounded? if 2 exit 1 ;

: 12swap ( a 2b - 2b a ) rot ;
: 21swap ( 2b a - a 2b ) -rot ;
: check-swap ( main-stack return-stack -- error? code )
	dup ts-swap dup 2 ts-pick type-width
	1 case dup 1 ts-pick type-width
		1 case 2drop & swap false exit
		2 case 2drop & 12swap false exit exit
	2 case dup 1 ts-pick type-width
		1 case 2drop & 21swap false exit
		2 case 2drop & 2swap false exit exit ;

: check-dup ( main-stack return-stack -- error? code )
	dup ts-dup dup 1 ts-pick type-width 
	1 case 2drop & dup false exit
	2 case 2drop & 2dup false exit ;

: 12over ( a 2b -- 2b a 2b ) >r 2dup r> rot ;
: 21over ( 2b a -- a 2b a ) -rot dup >r rot r> ;
: check-over ( main-stack return-stack -- error? code )
	dup ts-over dup 2 ts-pick type-width
	1 case dup 1 ts-pick type-width
		1 case 2drop & over false exit
		2 case 2drop & 12over false exit exit
	2 case dup 1 ts-pick type-width
		1 case 2drop & 21over false exit
		2 case 2drop & 2over false exit exit ;

: check-drop ( main-stack return-stack -- error? code )
	dup ts-pop type-width
	1 case 2drop & drop false exit
	2 case 2drop & 2drop false exit ;


( Default type checking code )
( Type checking )

: norm-ref ( type -- normalised-type ) 0 swap >ref dup ref-type
	ref-rel case ref-abs swap >ref-type exit drop ;
: type-neq ( type type -- neq ) norm-ref swap norm-ref != ;
: check-types ( n list list -- type-error? )
	0 case 2drop false exit 1 - >r
	dup 1 w + >r @ swap
	dup 1 w + >r @
	type-neq if r> r> r> 2drop ( n is error ) exit
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

( Interpreter wrappers )

: own-error ( ) " Ownership error!" puts 1 terminate ;
: own-check ( word -- error? ) 0 swap ( rel-bitmap )
	dup abs-claim @ ( abs-bitmap )
	swap rel-claim c@ ( arg-ref-bitmap )
	iter-main-stack 1 ts-pick-ref swap ( list )
	iter-main-stack top ( n )
	check-list-ownership ;

: print-type-list ( n list ) 0 case drop exit 1 - >r
	dup 1 w + >r @ print-type r> r> tail ;

: type-error ( error word -- ) over name print
	" : Types do not match: arg " print putx 10 putc
	iter-main-stack over args ts-pick-ref
	 over args print-type-list 10 putc
	dup types swap args print-type-list
	1 terminate ;
: type-check ( word -- error? ) 
	iter-main-stack over args ts-pick-ref 
	swap dup types swap args check-types ;

: height-error ( error word -- ) over name print
	" : Word is too high for the " print puts
	1 case " main stack" puts 
		" Main stack: " print iter-main-stack top putx
		" word: " print height-main putx then
	2 case " return stack" puts
		" Main stack: " print iter-ret-stack top putx
		" word: " print height-ret putx then
	terminate ;
: height-check ( word -- error? )
	dup height-main iter-main-stack top + 
	stack-size > if	1 exit
	height-ret iter-ret-stack top + 
	stack-size > if 2 exit
	false ;

: call-word ( word ) code call ;

: find-error ( name ) " Unable to find word: " print puts
	1 terminate ;

: default-type-check ( word  -- error? code )
	dup height-check dup if height-error exit
	dup type-check dup if type-error exit
	dup own-check if own-error exit
	code false ;

( Interpreter code )

: interpreter-sketch ( )
	next-word 0 case 2drop exit 2dup
	dict find 0 case find-error exit rot 2drop
	word-call-test if exit
	call-word tail ;
