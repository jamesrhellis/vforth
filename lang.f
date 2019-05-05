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
		e.g. : eg array @ 3 + ? 0 exit then @ ;
		: eg2 optional-owner @@ ? error exit then no-error ;
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
: types 4 w + ; ( Combined storage of return and call types )
: abs-claim 12 w + ;	( Global ownerships claimed - bitset )
: rel-claim 13 w + ;	( Ownerships from stack claimed - 8 bit list )
: height 14 w + ;	( Max height increase of stacks - 8 bit list )
			( 0 - main , 1 - ret , 2/3 reserved for fp / vec ) 
: height-main 14 w + ; 
: height-ret 14 w + 1 ;
: code 15 w + ; 	( Code - bounded pointer )

0 var dict !
: find ( word? name -- word? ) 0 case swap drop exit then
	dup >r name> string-eq if 2drop r> exit then
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
: items 1 w + ;

( Type manipulation words )
( type format :
	0-10 type
	11 optional? ; does this type need checking for validity
	12 bounded? ; is this a ref / own to an array
	13-14 ref-type : 0 -> not a ref, 1 -> ref to stack item,
		2 -> ref to global item, 3 -> ownership
	15-20 ref? ; what object is this a ref of / 0 -> own > ref
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
: ref-type 13 rshift ref-type-mask and ;
: >ref-type ( to type -- to ) ref-type-mask 13 lshift invert and swap
	ref-type-mask and 13 lshift or ;

0 invert 5 lshift invert con ref-mask
: ref 15 rshift ref-mask and ;
: >ref ( to own -- to ) ref-mask 15 lshift invert and swap
	ref-mask and 15 lshift or ;

( Type tracking state )

( Stack used for tracking types during interpretation )
17 w alloc con iter-main-stack
17 w alloc con iter-ret-stack
( Stack used for compiling )
17 w alloc con comp-main-stack
17 w alloc con comp-ret-stack
( Saved return state for if statements )
17 w alloc con if-main-stack
17 w alloc con if-ret-stack

( Type checking )

: set-bit ( bit -- bitmap ) 1 swap lshift ;
: list-ownership-bitmap ( n bitmap list -- bitmap ) 
	. 0 case drop swap drop exit then 1 - >r >r
	dup 1 w + swap @ dup ref-type 
	2 != if drop r> r> tail then
	ref set-bit . r> . or r> tail ;
: list-ownership-bitmap ( list n -- bitmap ) swap 0 swap list-ownership-bitmap ;
: stack-ownership-bitmap ( stack n -- bitmap ) items list-ownership-bitmap ;

: check-ownership ( stack word ) stack-ownership-bitmap swap abs-claim and ;