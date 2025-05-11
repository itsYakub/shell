#include "shell.h"

void	sh_free(struct s_shell *sh) {
	if (sh->input) {
		free(sh->input); sh->input = 0;
	}
	if (sh->tokens) {
		sh_free2d((void **) sh->tokens); sh->tokens = 0;
	}
}

void	sh_free2d(void **ptr) {
	void	**_ptr;

	_ptr = ptr;
	while (*_ptr)
		free(*_ptr++);
	if (ptr)
		free(ptr);
}

bool	sh_iskeyword(const char *cmd) {
	if (!strcmp(cmd, ";") ||
		!strcmp(cmd, "||") ||
		!strcmp(cmd, "|") ||
		!strcmp(cmd, "<") ||
		!strcmp(cmd, ">") ||
		!strcmp(cmd, ">>")
	) {
		return (false);
	}
	return (true);
}
