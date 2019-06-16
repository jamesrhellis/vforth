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

: dict-find ( name word? -- word? ) 0 case drop 0 exit
	dup >r
	name 2! 2over string-eq if 2drop r> exit
	r> next tail ;
: dict-find ( name -- word? ) dict @ dict-find ; 
