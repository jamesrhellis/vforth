struct module {
	struct module *next;
	char *name;
};
struct module *list = NULL;

int module_find(char *file_name) {
	struct module *p = list;
	while (p) {
		if (!strcmp(file_name, p->name)) {
			return 1;
		}
		p = p->next;
	}
	return 0;
}

char *strdup(const char *string) {
	size_t l = strlen(string) + 1;
	char *dup = malloc(l);
	memcpy(dup, string, l);

	return dup;
}

int module_find_add(char *file_name) {
	if (module_find(file_name)) {
		return 1;
	}

	struct module *m = malloc(sizeof(*m));
	*m = (struct module) {
		.next = list,
		.name = strdup(file_name),
	};
	list = m;

	return 0;
}

int load_forth_file(char *file_name, f_state *fs) {
	if (module_find_add(file_name)) {
		return 1;
	}
	char *bfile = file, *bmem = mem;
	load_file(file_name);

	interpreter(fs);

	free(mem);
	file = bfile;
	mem = bmem;

	return 0;
}

void finclude(f_state *fs) {
	char *fname = next_word();
	char buffer[256] = "./fsrc/";
	strcat(buffer + strlen(buffer), fname);
	load_forth_file(buffer, fs);
}

#include <dlfcn.h>
int load_binary_module(char *file_name) {
	if (module_find_add(file_name)) {
		return 1;
	}

	void *lib = dlopen(file_name, RTLD_LAZY | RTLD_GLOBAL);
	if (!lib) {
		fprintf(stderr, "Unable to load lib: %s;\n %s\n", file_name, dlerror());
		return 2;
	}
	void *lib_init = dlsym(lib, "lib_init");
	if (!lib_init) {
		fprintf(stderr, "Unable to initialise lib %s;\n %s\n", lib, dlerror());
		return 2;
	}
	((void (*)(void(*)(char *, syscall), void(*)(char *, void *)))lib_init)(add_syscall, add_ptr_lit);
	return 0;
}

void flibload(f_state *fs) {
	char *fname = next_word();
	load_binary_module(fname);
}
