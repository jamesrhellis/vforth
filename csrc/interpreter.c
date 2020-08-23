#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"

#define I(INS) #INS
char *ins_map[] = {
#include "ins.h"
};
#undef I

#define I(INS) INS
enum ins {
#include "ins.h"
};
#undef I

int syscalls_top = 0;
syscall syscalls[1024] = {0};

void interpret(f_state *fs) {
	stack *s = &fs->s;
	stack *ret = &fs->ret;
	ins **pc = &fs->pc;

	int base = ret->size;
	reg tmp = 0, tmp2 = 0;
	ins *p = *pc;
	while (p) {
		switch (*(p++)) {
		case I_SYS:
			tmp = 0;
			memcpy(&tmp, p, 2);

			ret->items[ret->size++] = ret->top;
			ret->top = (reg)(p + 2);
			*pc = p + 2;

			syscalls[tmp](fs);

			p = (ins *)ret->top;
			ret->top = ret->items[--ret->size];

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

		case I_TA:
			fs->a = s->top;
			s->top = s->items[--s->size];
			break;
		case I_FA:
			s->items[s->size++] = s->top;
			s->top = fs->a;
			break;
		case I_TB:
			fs->b = s->top;
			s->top = s->items[--s->size];
			break;
		case I_FB:
			s->items[s->size++] = s->top;
			s->top = fs->b;
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
			s->size -= 2;
			break;
		case I_LT:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 1] > s->items[s->size - 2] ? ~((reg)0) : 0;
			s->size -= 2;
			break;
		case I_EQ:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 1] == s->items[s->size - 2] ? ~((reg)0) : 0;
			s->size -= 2;
			break;
		case I_NE:
			s->items[s->size++] = s->top;
			s->top = s->items[s->size - 1] != s->items[s->size - 2] ? ~((reg)0) : 0;
			s->size -= 2;
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
		#define BRANCH {tmp = 0; memcpy(&tmp, p, 3); tmp |= tmp & 0x800000 ? ~(reg)0xFFFFFF : 0; p += tmp;}

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
			s->top = s->items[--s->size];
			break;
		case I_BNZ:
			if (s->top)
				BRANCH
			else
				SKIP
			s->top = s->items[--s->size];
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
			puts("Invalid Instruction!");
			exit(1);
		}
	}
	*pc = p;
}

#include "forth.c"

int main(int argn, char ** args) {
	if (argn < 2) {
		return -1;
	}

	load_file(args[1]);

	forth_init();

	f_state fs = {0};
	interpreter(&fs);

	free_words();
	free(mem);

	return 0;
}
