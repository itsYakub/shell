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

void	sh_close_fds(t_sh *sh) {
	close(sh->fd_pipe[0]);
	close(sh->fd_pipe[1]);
	close(sh->fd_null);
	close(sh->fd_curin);
	dup2(sh->fd_stdin, 0); close(sh->fd_stdin);
	dup2(sh->fd_stdout, 1); close(sh->fd_stdout);
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

char	*sh_getline(int fd) {
	char	*_tmp;
	char	*_dst;
	char	_c;

	_dst = _tmp = 0;
	_c = 0;
	if (read(fd, 0, 0) != -1) {
		read(fd, &_c, 1);
		if (!_c || _c == EOF) {
			return (0);
		}
		_dst = calloc(1, sizeof(char));
		if (!_dst) {
			perror("calloc");
			return (0);
		}
		do {
			_tmp = _dst;
			_dst = sh_strjoinc(_dst, _c);
			free(_tmp);
		} while (read(fd, &_c, 1) && _c != 0 && _c != '\n');
	}
	return (_dst);
}

char	*sh_strjoinc(char *s0, char c) {
	char	*_s;

	_s = (char *) calloc(strlen(s0) + 2, sizeof(char));
	if (!_s) {
		perror("calloc");
		return (0);
	}
	_s = strcat(_s, s0);
	_s = strncat(_s, &c, 1);
	return (_s);
}
