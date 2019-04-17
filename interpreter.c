#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef uint8_t ins;
typedef size_t reg;

typedef struct stack {
	int size;
	reg top;
	reg items[16];
} stack;

static inline int stack_push(stack *s, reg item) {
	if (s->size >= 16) {
		return 1;
	}
	s->items[s->size++] = s->top;
	s->top = item;
	return 0;
}

static inline reg stack_pop(stack *s) {
	reg tmp = s->top;
	s->top = s->items[--s->size];
	return tmp;
}

char *ins_map[] = {
	"I_SYS",	// Perform a call to hosting environment
	// Arithmetic
	"I_ADD",
	"I_SUB",

	"I_LSL",
	"I_LSR",

	// Stack Operations
	"I_SWAP",
	"I_ESWAP",
	"I_DROP",
	"I_OVER",
	"I_EOVER",
	"I_DUP",
	"I_EDUP",
	"I_ROT",
	"I_NROT",

	"I_TR",
	"I_FR",

	// Data
	"I_ZERO",
	"I_IMM8",
	"I_IMMW",

	"I_BLD",
	"I_LD",
	"I_ELD",
	"I_BST",
	"I_ST",
	"I_EST",

	// Binary
	"I_AND",
	"I_NOT",
	"I_OR",
	"I_XOR",

	// Comparison
	"I_GT",
	"I_LT",
	"I_EQ",
	"I_NE",

	// Branching
	"I_BLT",
	"I_BGT",
	"I_BZ",
	"I_BNZ",
	"I_B",

	"I_BL",
	"I_BNL",

	"I_RET",

	"NO_I",
};

enum ins {
	I_SYS,	// Perform a call to hosting environment
	// Arithmetic
	I_ADD,
	I_SUB,

	I_LSL,
	I_LSR,

	// Stack Operations
	I_SWAP,
	I_ESWAP,
	I_DROP,
	I_OVER,
	I_EOVER,
	I_DUP,
	I_EDUP,
	I_ROT,
	I_NROT,

	I_TR,
	I_FR,

	// Data
	I_ZERO,
	I_IMM8,
	I_IMMW,

	I_BLD,
	I_LD,
	I_ELD,
	I_BST,
	I_ST,
	I_EST,

	// Binary
	I_AND,
	I_NOT,
	I_OR,
	I_XOR,

	// Comparison
	I_GT,
	I_LT,
	I_EQ,
	I_NE,

	// Branching
	I_BLT,
	I_BGT,
	I_BZ,
	I_BNZ,
	I_B,

	I_BL,
	I_BNL,

	I_RET,

	NO_I,
};

#define STATE ins **pc, stack *s, stack *ret

typedef void (* syscall)(STATE);

void fexit(STATE) {
	*pc = NULL;
}

void test(STATE) {
	printf("%d", s->top);
}

void falloc(STATE) {
	size_t len = s->top;
	s->top = (size_t) calloc(1, s->top);
	stack_push(s, s->top + len);
}

void ffree(STATE) {
	free((void *)stack_pop(s));
}

void colon(STATE);

syscall syscalls[1024] = {
	fexit,
	colon,
	falloc,
	ffree,
};

void interpret(STATE) {
	int base = ret->size;
	reg tmp = 0, tmp2 = 0;
	ins *p = *pc;
	while (p) {
		switch (*(p++)) {
		case I_SYS:
			tmp = 0;
			memcpy(&tmp, p, 2);
			p += 2;

			syscalls[tmp](&p, s, ret);

			break;
		case I_ADD:
			s->top = s->items[--s->size] + s->top;
			break;
		case I_SUB:
			s->top = s->items[--s->size] - s->top;
			break;
		case I_LSL:
			s->top = s->items[--s->size] << s->top;
			break;
		case I_LSR:
			s->top = s->items[--s->size] >> s->top;
			break;

		case I_ESWAP:
			tmp = s->top;
			tmp2 = s->items[s->size -  1];
			s->top = s->items[s->size - 2];
			s->items[s->size - 1] = s->items[s->size - 3];
			s->items[s->size - 2] = tmp;
			s->items[s->size - 3] = tmp2;
			break;
		case I_SWAP:
			tmp = s->top;
			s->top = s->items[s->size - 1];
			s->items[s->size - 1] = tmp;
			break;

		case I_DROP:
			s->top = s->items[--s->size];
			break;

		case I_EOVER:
			s->items[s->size] = s->top;
			s->items[s->size + 1] = s->items[s->size - 3];
			s->top = s->items[s->size - 2];
			s->size += 2;
			break;
		case I_OVER:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 2];
			break;

		case I_EDUP:
			s->items[s->size++] = s->top;
			s->items[s->size] = s->items[s->size - 2];
			break;
		case I_DUP:
			s->items[s->size++] = s->top;
			break;

		case I_ROT:
			tmp = s->items[s->size - 2];
			s->items[s->size - 2] = s->items[s->size - 1];
			s->items[s->size - 1] = s->top;
			s->top = tmp;
			break;

		case I_NROT:
			tmp = s->top;
			s->top = s->items[s->size - 1];
			s->items[s->size - 1] = s->items[s->size - 2];
			s->items[s->size - 2] = tmp;
			break;

		case I_TR:
			ret->items[ret->size++] = ret->top;
			ret->top = s->top;
			s->top = s->items[--s->size];
			break;
		case I_FR:
			s->items[s->size++] = s->top;
			s->top = ret->top;
			ret->top = ret->items[--ret->size];
			break;

		case I_ZERO:
			s->items[s->size++] = s->top;
			s->top = 0;
			break;
		case I_IMM8:
			s->items[s->size++] = s->top;
			s->top = *(p++);
			break;
		case I_IMMW:
			s->items[s->size++] = s->top;
			s->top = 0;
			memcpy(&s->top, p, sizeof(size_t));
			p += sizeof(size_t);
			break;

		case I_ELD:
			s->items[s->size++] = *((reg *)s->top + 1);
		case I_LD:
			s->top = *(reg *)s->top;
			break;
		case I_BLD:
			s->top = *(uint8_t *)s->top;
			break;
		case I_EST:
			*((reg *)s->top + 1) = s->items[s->size-2];
			*(reg *)s->top = s->items[s->size-1];
			s->top = s->items[(s->size-=3)];
			break;
		case I_ST:
			*(reg *)s->top = s->items[--s->size];
			s->top = s->items[--s->size];
			break;
		case I_BST:
			*(uint8_t *)s->top = s->items[--s->size];
			s->top = s->items[--s->size];
			break;
			
			
		case I_GT:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 1] < s->items[s->size - 2] ? ~((reg)0) : 0;
			break;
		case I_LT:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 1] > s->items[s->size - 2] ? ~((reg)0) : 0;
			break;
		case I_EQ:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 1] == s->items[s->size - 2] ? ~((reg)0) : 0;
			break;
		case I_NE:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 1] != s->items[s->size - 2] ? ~((reg)0) : 0;
			break;

		case I_AND:
			s->top &= s->items[--s->size];
			break;
		case I_OR:
			s->top |= s->items[--s->size];
			break;
		case I_XOR:
			s->top ^= s->items[--s->size];
			break;
		case I_NOT:
			s->top = ~s->top;
			break;

		#define SKIP {p += 3;}
		#define BRANCH {memcpy(&tmp, p, 3); tmp |= tmp & 0x800000 ? ~(reg)0xFFFFFF : 0; p += tmp;}

		case I_BGT:
			if (s->items[s->size - 1] > s->top)
				BRANCH
			else
				SKIP
			break;
		case I_BLT:
			if (s->items[s->size - 1] < s->top)
				BRANCH
			else
				SKIP
			break;
		case I_BZ:
			if (!s->top)
				BRANCH
			else
				SKIP
			break;
		case I_BNZ:
			if (s->top)
				BRANCH
			else
				SKIP
			break;
		case I_B:
			BRANCH
			break;

		case I_BL:
			ret->items[ret->size++] = ret->top;
			ret->top = (size_t)p + sizeof(size_t);

			memcpy(&p, p, sizeof(size_t));
			break;
		case I_BNL:
			memcpy(&p, p, sizeof(size_t));
			break;
		case I_RET:
			if (ret->size == base) {
				p = 0;
			} else {
				p = (ins *)ret->top;
				ret->top = ret->items[--ret->size];
			}
			break;

		default:
			break;
		}
	}
	*pc = p;
}

#include "forth.c"

int size_pow(int n) {
	int c = 0;
	while (n >> ++c) {
	}

	return c - 1;
}

int main(int argn, char ** args) {
	if (argn < 2) {
		return -1;
	}

	load_file(args[1]);

	// Basic words
	add_word("+", 1, (ins []){I_ADD}); inlin();
	add_word("-", 1, (ins []){I_SUB}); inlin();
	add_word("lshift", 1, (ins []){I_LSL}); inlin();
	add_word("rshift", 1, (ins []){I_LSR}); inlin();
	add_word("and", 1, (ins []){I_AND}); inlin();
	add_word("or", 1, (ins []){I_OR}); inlin();
	add_word("eor", 1, (ins []){I_XOR}); inlin();
	add_word("invert", 1, (ins []){I_NOT}); inlin();
	add_word("swap", 1, (ins []){I_SWAP}); inlin();
	add_word("2swap", 1, (ins []){I_ESWAP}); inlin();
	add_word("drop", 1, (ins []){I_DROP}); inlin();
	add_word("dup", 1, (ins []){I_DUP}); inlin();
	add_word("2dup", 1, (ins []){I_EDUP}); inlin();
	add_word("over", 1, (ins []){I_OVER}); inlin();
	add_word("2over", 1, (ins []){I_EOVER}); inlin();
	add_word("rot", 1, (ins []){I_ROT}); inlin();
	add_word("-rot", 1, (ins []){I_NROT}); inlin();
	add_word(">r", 1, (ins []){I_TR}); inlin();
	add_word("r>", 1, (ins []){I_FR}); inlin();
	add_word("0", 1, (ins []){I_ZERO}); inlin();
	add_word("c!", 1, (ins []){I_BST}); inlin();
	add_word("!", 1, (ins []){I_ST}); inlin();
	add_word("2!", 1, (ins []){I_EST}); inlin();
	add_word("c@", 1, (ins []){I_BLD}); inlin();
	add_word("@", 1, (ins []){I_LD}); inlin();
	add_word("2@", 1, (ins []){I_ELD}); inlin();
	add_word(">", 1, (ins []){I_GT}); inlin();
	add_word("<", 1, (ins []){I_LT}); inlin();
	add_word("=", 1, (ins []){I_EQ}); inlin();
	add_word("!=", 1, (ins []){I_NE}); inlin();
	add_word("exit", 1, (ins []){I_RET}); inlin();
	add_word(":", 3, (ins []) {I_SYS, 1, 0});

	// Utility words
	add_word("w", 3 , (ins []) {I_IMM8, size_pow(sizeof(size_t)), I_LSL}); inlin();
	add_word("i-b", 2, (ins []) {I_IMM8, I_B}); inlin();
	add_word("i-bz", 2, (ins []) {I_IMM8, I_BZ}); inlin();

	// Forth interpreter state interaction
	ins tmp[1 + sizeof(size_t)] = {I_IMMW};
	void *ptmp = &pos;
	memcpy(&tmp[1], &ptmp, sizeof(size_t));
	add_word("buffer-pos", 1 + sizeof(size_t), tmp);
	ptmp = &buffer;
	memcpy(&tmp[1], &ptmp, sizeof(size_t));
	add_word("buffer-items", 1 + sizeof(size_t), tmp);
	ptmp = &dict;
	memcpy(&tmp[1], &ptmp, sizeof(size_t));
	add_word("dict", 1 + sizeof(size_t), tmp);
	ptmp = &file;
	memcpy(&tmp[1], &ptmp, sizeof(size_t));
	add_word("in-file", 1 + sizeof(size_t), tmp);

	add_word("alloc", 3, (ins []) {I_SYS, 2, 0}); inlin();
	add_word("free", 3, (ins []) {I_SYS, 3, 0}); inlin();
	
	stack s = {0};
	stack ret = {0};
	ins *pc = NULL; //(ins []){I_IMM8, 12, I_IMM8, 13, I_ADD, I_SYS, 0, 0};
	//interpret(&pc, &s, &ret);
	interpreter(&pc, &s, &ret);
	
	printf("%d\n", s.top);
	return 0;
}
