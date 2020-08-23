#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

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

typedef struct {
	ins *pc;
	stack s;
	stack ret;

	reg a;
	reg b;
} f_state;

typedef void (* syscall)(f_state *fs) ;
typedef void (* syscall_register)(char *name, syscall) ;
