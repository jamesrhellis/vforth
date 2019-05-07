typedef struct word {
	struct word *next;
	char *name;
	size_t len;
	size_t flags;
	ins code[];
} word;

enum w_flags {
	W_IMMIDIATE = 1,
	W_INLINE = 2,
};

word *dict = NULL;

void add_word(char *name, int len, ins *code) {
	char *n = malloc(strlen(name) + 1);
	memcpy(n, name, strlen(name) + 1);
	word *w = malloc(sizeof(word) + sizeof(ins) * (len + 1));
	*w = (word) {
		.next = dict,
		.name = n,
		.len = len,
	};
	memcpy(w->code, code, len * sizeof(ins));
	w->code[len] = I_RET;
	dict = w;
}

word *find_word(char *name) {
	word *p = dict;
	while (p) {
		if (!strcmp(p->name, name)) {
			return p;
		}
		p = p->next;
	}

	return NULL;
}

void free_words(void) {
	word *w = dict;
	while (w) {
		free(w->name);
		word *tmp = w;
		w = w->next;
		free(tmp);
	}

	return;
}

char *file = NULL, *mem = NULL;

int load_file(char *file_name) {
	FILE *f = fopen(file_name, "rb");
	if (!f) {
		return 1;
	}

	if (fseek(f, 0, SEEK_END)) {
		fclose(f);
		return 2;
	}
	size_t end = ftell(f);
	rewind(f);

	char *fi = malloc(end + 1);
	if (end != fread(fi, 1, end, f)) {
		fclose(f);
		return 3;
	}
	fi[end] = 0;

	file = fi;
	mem = fi;
	fclose(f);
	return 0;
}
	
char *next_word(void) {
	if (!file || !*file) {
		return NULL;
	}

	while (isspace(*file)) {
		++file;
	}

	if (!*file) {
		return NULL;
	}

	char *start = file;
	while (isgraph(*file)) {
		++file;
	}

	if (*file) {
		*(file++) = 0;
	}

	return start;
}

void inlin(void) {
	dict->flags |= W_INLINE;
}
void imm(void) {
	dict->flags |= W_IMMIDIATE;
}

size_t pos;
ins buffer[1024];

void colon(STATE) {
	pos = 0;
	char *name = next_word();
	char *c;
	while((c = next_word())) {
		if (!strcmp(c, ";")) {
			break;
		}

		word *w = find_word(c);
		if (w) {
			if (w->flags & W_IMMIDIATE) {
				*pc = w->code;
				interpret(pc, s, ret);
			} else if (w->flags & W_INLINE) {
				memcpy(&buffer[pos], &w->code, w->len);
				pos += w->len;
			} else {
				buffer[pos++] = I_BL;
				void *tmp = &w->code;
				memcpy(&buffer[pos], &tmp, sizeof(size_t));
				pos += sizeof(size_t);
			}
		} else {
			char *end = NULL;
			size_t l = strtol(c, &end, 0);
			if (*end || l == 0) {
				fprintf(stderr, "Unknown word: %s\n", c);
				exit(-1);
			} else if (l < 256) {
				buffer[pos++] = I_IMM8;
				buffer[pos++] = l;
			} else {
				buffer[pos++] = I_IMMW;
				memcpy(&buffer[pos], &l, sizeof(size_t));
				pos += sizeof(size_t);
			}
		}
	}

	add_word(name, pos, buffer);
}

void *var_space[3];

void interpreter(STATE) {
	char *c;
	while((c = next_word())) {
		word *w = find_word(c);
		if (w) {
			*pc = w->code;
			interpret(pc, s, ret);
		} else {
			char *end = NULL;
			size_t l = strtol(c, &end, 0);
			if (*end || l == 0) {
				fprintf(stderr, "Unknown word: %s\n", c);
				exit(-1);
			} else {
				stack_push(s, l);
				
			}
		}
	}
}

void finclude(STATE) {
	char *fname = next_word();
	char *bfile = file, *bmem = mem;
	load_file(fname);

	interpreter(pc, s, ret);

	free(mem);
	file = bfile;
	mem = bmem;
}

void falloc(STATE) {
	size_t len = stack_pop(s);
	void *m = calloc(1, len);
	stack_push(s, ((size_t)m) + len);
	stack_push(s, (size_t) m);
}

void ffree(STATE) {
	free((void *)stack_pop(s));
}

void print_stack(STATE) {
	printf("%x, ", s->top); 
	for (int i = s->size -1;i >= 0;--i) {
		printf("%x, ", s->items[i]); 
	}
	puts("");
}

void ffind_word(STATE) {
	char *c = next_word();
	if (!c) {
		exit(-1);
	}
	word *w = find_word(c);
	if (!w) {
		fprintf(stderr, "Unknown word: %s\n", c);
		exit(-1);
	}
	stack_push(s, (size_t)w->code);
}


void add_syscall(char *name, syscall s) {
	int no = syscalls_top++;
	syscalls[no] = s;
	add_word(name, 3, (ins []) {I_SYS, no & 0xF, (no >> 8) & 0xF}); 
}

#include <dlfcn.h>
void flibload(STATE) {
	char *fname = next_word();
	void *lib = dlopen(fname, RTLD_LAZY | RTLD_GLOBAL);
	if (!lib) {
		fprintf(stderr, "Unable to load lib: %s;\n %s\n", fname, dlerror());
		return;
	}
	void *lib_init = dlsym(lib, "lib_init");
	if (!lib_init) {
		fprintf(stderr, "Unable to initialise lib %s;\n %s\n", lib, dlerror());
		return;
	}
	((void (*)(void(*)(char *, syscall)))lib_init)(add_syscall);
}

void add_syscalls() {
	add_syscall("in", finclude);
	add_syscall(":", colon);
	add_syscall(".", print_stack);
	add_syscall("alloc", falloc); inlin();
	add_syscall("free", ffree); inlin();
	add_syscall("load", flibload); inlin();
	add_syscall("&", ffind_word);
}

int size_pow(int n) {
	int c = 0;
	while (n >> ++c) {
	}

	return c - 1;
}

void add_base_words() {
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
	add_word("2drop", 2, (ins []){I_DROP, I_DROP}); inlin();
	add_word("dup", 1, (ins []){I_DUP}); inlin();
	add_word("2dup", 2, (ins []){I_OVER,I_OVER}); inlin();
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

	// Utility words
	add_word("w", 3 , (ins []) {I_IMM8, size_pow(sizeof(size_t)), I_LSL}); inlin();

	for (int i = 0;i < NO_I;++i) {
		add_word(ins_map[i], 2, (ins []) {I_IMM8, i}); inlin();
	}

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
	ptmp = &var_space;
	memcpy(&tmp[1], &ptmp, sizeof(size_t));
	add_word("var-space", 1 + sizeof(size_t), tmp);
}

void forth_init(void) {
	add_syscalls();
	add_base_words();
}
