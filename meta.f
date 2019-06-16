in base.f

( This module provides words to interact with the forth interpreter state:
	finding dictionary words
)

( Dictionary words )
( Word words )
: next ;		( next word in dictionary )
: name 1 w + ;		( name of the word: \0 terminated and bounded pointer )
: len 3 w + ; 		( length of the instruction sequence )
: flags 4 w + ;		( immediate 0, inline 1 )
: code 5 w + ;		( executable intructions inline )

: dict-find ( name word? -- name word? ) 0 case 0 exit
	dup >r
	name 2! 2over string-eq if r> exit
	r> next tail ;
: dict-find ( name -- name word? ) dict @ dict-find ; 

( Postpone equivilent )
: ' next-word dict-find
	0 case " Unable to find word: " print puts 1 terminate exit
	code I_CALL buffer-push buffer-pushw 2drop ; 

( Add wrapper for & to inline found word address )
: & & I_IMMW buffer-push buffer-pushw ; imm
