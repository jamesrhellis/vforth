#include "types.h"
#include <stdio.h>

/* C stdio wrapper functions */
void f_fopen(f_state *fs) {
	reg mode = stack_pop(&fs->s);
	reg filename = stack_pop(&fs->s);

	stack_push(&fs->s, (reg)fopen((char *)filename, (char *)mode));
}

void f_fclose(f_state *fs) {
	reg file = stack_pop(&fs->s);
	fclose((FILE *)file);
}

void f_fread(f_state *fs) {
	reg file = stack_pop(&fs->s);
	reg size = stack_pop(&fs->s);
	reg ptr = stack_pop(&fs->s);

	stack_push(&fs->s, fread((void *)ptr, 1, size, (FILE *)file));
}

void f_fwrite(f_state *fs) {
	reg file = stack_pop(&fs->s);
	reg size = stack_pop(&fs->s);
	reg ptr = stack_pop(&fs->s);

	stack_push(&fs->s, fwrite((void *)ptr, 1, size, (FILE *)file));
}

// Custom function to get the size of a file
void f_fsize(f_state *fs) {
	FILE *file = (FILE *)stack_pop(&fs->s);

	if (!fseek(file, 0, SEEK_END)) {
		return;
	}

	stack_push(&fs->s, ftell(file));

	rewind(file);
}

void f_fputs(f_state *fs) {
	reg file = stack_pop(&fs->s);
	reg str = stack_pop(&fs->s);

	stack_push(&fs->s, fputs((char *)str, (FILE *)file));
}

void f_fgets(f_state *fs) {
	reg file = stack_pop(&fs->s);
	reg num = stack_pop(&fs->s);
	reg str = stack_pop(&fs->s);

	stack_push(&fs->s, (reg)fgets((char *)str, num, (FILE *)file));
}

void f_fputc(f_state *fs) {
	reg file = stack_pop(&fs->s);
	reg ch = stack_pop(&fs->s);

	fputc(ch, (FILE *)file);
}

void f_fgetc(f_state *fs) {
	reg file = stack_pop(&fs->s);

	stack_push(&fs->s, fgetc((FILE *)file));
}

void lib_init(syscall_register add, literal_register ladd) {
	add("fopen", f_fopen);
	add("fclose", f_fclose);
	add("fread", f_fread);
	add("fwrite", f_fwrite);
	add("fsize", f_fsize);
	add("fputs", f_fputs);
	add("fgets", f_fgets);
	add("fputc", f_fputc);
	add("fgetc", f_fgetc);

	ladd("stdin", stdin);
	ladd("stdout", stdout);
}
