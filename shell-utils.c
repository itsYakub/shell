#include "shell.h"

void	sh_free2d(void **ptr) {
	void	**_ptr;

	_ptr = ptr;
	while (*_ptr)
		free(*_ptr++);
	if (ptr)
		free(ptr);
}

void	sh_clear_fbuf(FILE *f) {
	int	_c;

	_c = 0;
	while ((_c = fgetc(f)) != '\n' && _c != EOF)
		continue;
}
