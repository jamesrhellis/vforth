in base.f
( This module provides io functionality:
	printing )

( Printing functions )

: print ( string -- ) dup c@ 0 case exit putc 1 + tail ;

: puts ( string -- ) print 10 putc ;

: char in-file @ dup c@ swap 1 + in-file !
	I_IMM8 buffer-push buffer-push ; imm

: hex-char ( no -- char ) dup 10 < if char 0 + exit
	10 - char a + ;
15 con hex-mask
: hex-mask hex-mask and ;
: putx ( number shift -- ) 0 case drop exit
	4 - over over rshift hex-mask hex-char putc tail ;
( outer loop to skip leading zeros )
: putx ( number shift ) 0 case char 0 putc exit
	4 - over over rshift hex-mask if 4 + putx exit tail ;
: putx ( number -- ) 8 w putx ;

( Testing facilities )
: assert ( cond string ) swap 0 = if puts 1 terminate then ;
