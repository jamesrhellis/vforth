in base.f
in io.f

( This module provides words to interact with the forth interpreter state:
	finding dictionary words )

( Dictionary words )
( Word words )
: word-next ;			( next word in dictionary )
: word-name 1 w + ;		( name of the word: \0 terminated and bounded pointer )
: word-len 3 w + ; 		( length of the instruction sequence )
: word-flags 4 w + ;		( immediate 0, inline 1 )
: word-code 5 w + ;		( executable intructions inline )

( Find word )
: dict-find ( name word? -- name word? ) 0 case 0 exit
	dup >r
	word-name 2@ 2over string-eq if r> exit
	r> word-next @ tail ;
: dict-find ( name -- name word? ) dict @ dict-find ; 

: dict-find-or-fail ( name -- name word ) dict-find
	0 case " Unable to find word: " print puts 1 terminate exit ;

( Postpone equivilent )
: ' next-word dict-find-or-fail
	word-code I_BL buffer-push buffer-pushw 2drop ; imm

( Get address of word )
: & next-word dict-find-or-fail
	word-code I_IMMW buffer-push buffer-pushw 2drop ; imm
