typedef struct word {
	struct word *next;
	char *name;
	size_t len, flags;
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

char *file = NULL;

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

	char *start = file;
	while (!isspace(*file)) {
		++file;
	}

	if (*file) {
		*(file++) = 0;
	}

	return start;
}

void push_back(char *word) {
	if (file) {
		*(file - 1) = ' ';
	}
	file = word;
}

void inlin(void) {
	dict->flags &= W_IMMIDIATE;
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
			size_t l = atol(c);
			if (l < 256) {
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

void interpreter(STATE) {
	char *c;
	while((c = next_word())) {
		word *w = find_word(c);
		if (w) {
			*pc = w->code;
			interpret(pc, s, ret);
		} else {
			stack_push(s, atol(c));
		}
	}
}
