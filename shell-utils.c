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
	if (!strcmp(cmd, ";")	||
		!strcmp(cmd, "&&")	||
		!strcmp(cmd, "|")	||
		!strcmp(cmd, "<")	||
		!strcmp(cmd, ">")	||
		!strcmp(cmd, ">>")
	) {
		return (false);
	}
	return (true);
}

bool	sh_isdelim(const char *cmd) {
	if (!strcmp(cmd, ";")	||
		!strcmp(cmd, "&&")	||
		!strcmp(cmd, "|") 
	) {
		return (true);
	}
	return (false);
}

int	sh_exec(t_sh *sh, char **av) {
	char	**_avcp;

	_avcp = av;
	while (*_avcp && sh_iskeyword(*_avcp)) {
		_avcp++;
	}
	*_avcp = 0;
	if (sh_isbltin_exec(*av)) {
		/* builtin: type */
		if (!strcmp(*av, "type")) {
			sh_bltin_type(sh, sh->tokens);
		}
		/* builtin: pwd */
		else if (!strcmp(*av, "pwd")) {
			sh_bltin_pwd(sh->tokens);
		}
		/* builtin: env */
		else if (!strcmp(*av, "env")) {
			sh_bltin_env(sh->tokens);
		}
	}
	else {
		execvp(*av, av);
		perror("execvp");
	}
	exit(1);
}
