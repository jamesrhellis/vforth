#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t ins;
typedef size_t reg;

typedef struct stack {
	int size;
	reg top;
	reg items[16];
} stack;

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
	I_OVER,
	I_EOVER,
	I_DUP,
	I_EDUP,
	I_ROT,
	I_EROT,
	I_NROT,
	I_ENROT,

	I_TR,
	I_ETR,
	I_FR,
	I_EFR,

	// Data
	I_ZERO,
	I_EZERO,
	I_IMM8,
	I_EIMM8,
	I_IMM32,
	I_EIMM32,

	I_LD,
	I_ELD,
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
	I_EBLT,
	I_BGT,
	I_EBGT,
	I_BZ,
	I_BNZ,

	I_BL,
	I_BNL,

	NO_I,
};

typedef void (* syscall)(ins **pc, stack *s, stack *ret, stack *sup);

void exit(ins **pc, stack *s, stack *ret, stack *sup){
	*pc = NULL;
}
void test(ins **pc, stack *s, stack *ret, stack *sup){
	printf("%d", s->top);
}

syscall syscalls[1024] = {
	exit,
	test,
};

int main(int argn, char ** args) {
	if (argn < 2) {
		return -1;
	}
	
	stack s = {0};
	stack ret = {0};
	stack sup = {0};
	reg tmp = 0;
	ins *pc = (ins []){I_IMM8, 12, I_IMM8, 13, I_ADD, I_SYS, 1, 0, I_SYS, 0, 0};
	while (pc) {
		switch (*(pc++)) {
		case I_SYS:
			tmp = 0;
			memcpy(&tmp, pc, 2);
			pc += 2;

			syscalls[tmp](&pc, &s, &ret, &sup);

			break;
		case I_ADD:
			s.top = s.items[--s.size] + s.top;
			break;
		case I_SUB:
			s.top = s.items[--s.size] - s.top;
			break;
		case I_LSL:
			s.top = s.items[--s.size] << s.top;
			break;
		case I_LSR:
			s.top = s.items[--s.size] << s.top;
			break;

		case I_ESWAP:
			tmp = sup.top;
			sup.top = sup.items[sup.size - 1];
			sup.items[sup.size - 1] = tmp;
		case I_SWAP:
			tmp = s.top;
			s.top = s.items[s.size - 1];
			s.items[s.size - 1] = tmp;
			break;

		case I_EOVER:
			sup.items[sup.size++] = sup.top;
			sup.top = sup.items[sup.size - 2];
		case I_OVER:
			s.items[s.size++] = s.top;
			s.top = s.items[s.size - 2];
			break;

		case I_EDUP:
			sup.items[sup.size++] = sup.top;
		case I_DUP:
			s.items[s.size++] = s.top;
			break;

		case I_EROT:
			tmp = sup.items[sup.size - 2];
			sup.items[sup.size - 2] = sup.items[sup.size - 1];
			sup.items[sup.size - 1] = sup.top;
			sup.top = tmp;
		case I_ROT:
			tmp = s.items[s.size - 2];
			s.items[s.size - 2] = s.items[s.size - 1];
			s.items[s.size - 1] = s.top;
			s.top = tmp;
			break;

		case I_ENROT:
			tmp = sup.top;
			sup.top = sup.items[sup.size - 1];
			sup.items[sup.size - 1] = sup.items[sup.size - 2];
			sup.items[sup.size - 2] = tmp;
			break;
		case I_NROT:
			tmp = s.top;
			s.top = s.items[s.size - 1];
			s.items[s.size - 1] = s.items[s.size - 2];
			s.items[s.size - 2] = tmp;
			break;

		case I_ETR:
			ret.items[ret.size++] = ret.top;
			ret.top = sup.top;
			sup.top = sup.items[--sup.size];
		case I_TR:
			ret.items[ret.size++] = ret.top;
			ret.top = s.top;
			s.top = s.items[--s.size];
			break;
		case I_EFR:
			s.items[s.size++] = s.top;
			s.top = ret.top;
			ret.top = ret.items[--ret.size];

			sup.items[sup.size++] = sup.top;
			sup.top = ret.top;
			ret.top = ret.items[--ret.size];

			break;
		case I_FR:
			s.items[s.size++] = s.top;
			s.top = ret.top;
			ret.top = ret.items[--ret.size];
			break;

		case I_ZERO:
			s.items[s.size++] = s.top;
			s.top = 0;
			break;
		case I_EZERO:
			sup.items[sup.size++] = sup.top;
			sup.top = 0;
			break;
		case I_IMM8:
			s.items[s.size++] = s.top;
			s.top = *(pc++);
			break;
		case I_EIMM8:
			sup.items[sup.size++] = sup.top;
			sup.top = *(pc++);
			break;
		case I_IMM32:
			s.items[s.size++] = s.top;
			s.top = 0;
			memcpy(&s.top, pc, 4);
			pc += 4;
			break;
		case I_EIMM32:
			sup.items[sup.size++] = sup.top;
			sup.top = 0;
			memcpy(&sup.top, pc, 4);
			pc += 4;
			break;
			
		case I_GT:
			s.items[s.size++] = s.top;
			s.top = s.items[s.size - 1] < s.items[s.size - 2] ? ~((reg)0) : 0;
			break;
		case I_LT:
			s.items[s.size++] = s.top;
			s.top = s.items[s.size - 1] > s.items[s.size - 2] ? ~((reg)0) : 0;
			break;
		case I_EQ:
			s.items[s.size++] = s.top;
			s.top = s.items[s.size - 1] == s.items[s.size - 2] ? ~((reg)0) : 0;
			break;
		case I_NE:
			s.items[s.size++] = s.top;
			s.top = s.items[s.size - 1] != s.items[s.size - 2] ? ~((reg)0) : 0;
			break;

		case I_AND:
			s.top &= s.items[--s.size];
			break;
		case I_OR:
			s.top |= s.items[--s.size];
			break;
		case I_XOR:
			s.top ^= s.items[--s.size];
			break;
		case I_NOT:
			s.top = ~s.top;
			break;

		#define SKIP {pc += 3;}
		#define BRANCH {memcpy(&tmp, pc, 3); tmp |= tmp & 0x800000 ? ~(reg)0xFFFFFF : 0; pc += tmp;}

		case I_EBGT:
			if (s.top > sup.top)
				BRANCH
			else
				SKIP
			break;
		case I_BGT:
			if (s.items[s.size - 1] > s.top)
				BRANCH
			else
				SKIP
			break;
		case I_EBLT:
			if (s.top < sup.top)
				BRANCH
			else
				SKIP
			break;
		case I_BLT:
			if (s.items[s.size - 1] < s.top)
				BRANCH
			else
				SKIP
			break;
		case I_BZ:
			if (s.items[s.size - 1] == s.top)
				BRANCH
			else
				SKIP
			break;
		case I_BNZ:
			if (s.items[s.size - 1] != s.top)
				BRANCH
			else
				SKIP
			break;

		case I_BL:
			ret.items[ret.size++] = ret.top;
			ret.top = (size_t)pc + sizeof size_t;

			memcpy(&pc, pc, sizeof size_t);
			break;
		case I_BNL:
			memcpy(&pc, pc, sizeof size_t);
			break;

		default:
			break;
		}
	}

	printf("%d\n", NO_I);
	return 0;
}
