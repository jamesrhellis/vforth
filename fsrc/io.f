in base.f
( This module provides io functionality:
	printing )

( Printing functions )

: print ( string -- ) 2dup <= if 2drop exit
	dup c@ putc 1 + tail ;

: puts ( string -- ) print 10 putc ;

: char in-file @ dup c@ swap 1 + in-file ! 
	I_IMM8 buffer-push buffer-push ; imm

1 w 3 lshift 4 - con hex-shift
: hex-shift ( number -- number ) hex-shift rshift ;
: hex-char ( number ) 
	dup 9 > if 10 - char a + exit
	char 0 + ;
: putx ( number -- ) 0 case exit
	dup hex-shift hex-char putc
	4 lshift tail ; 
( This is an outer loop to skip leading zeros )
: putx ( number -- ) 0 case char 0 putc exit
	dup hex-shift if putx exit
	4 lshift tail ;

( Testing facilities )
: assert ( cond string ) rot 0 = if puts 1 terminate then ;
