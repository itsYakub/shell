#include "shell.h"
#include <sys/param.h>

void	sh_free_input(struct s_shell *sh) {
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
	while (*_ptr) {
		free(*_ptr++);
	}
	if (ptr)
		free(ptr);
}

void	sh_close_fds(t_sh *sh) {
	close(sh->fd_pipe[0]); sh->fd_pipe[0] = -1;
	close(sh->fd_pipe[1]); sh->fd_pipe[1] = -1;
	close(sh->fd_null); sh->fd_null = -1;
	close(sh->fd_curin); sh->fd_curin = -1;
	dup2(sh->fd_stdin, 0); close(sh->fd_stdin); sh->fd_stdin = -1;
	dup2(sh->fd_stdout, 1); close(sh->fd_stdout); sh->fd_stdout = -1;
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
		if (read(fd, &_c, 1) == -1) {
			return (0);
		}
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
	_s[strlen(_s)] = c;
	_s[strlen(_s)] = 0;
	return (_s);
}

char	*sh_expstr(struct s_shell *sh, const char *t) {
	char	_result[1024];
	char	_varname[1024];
	char	*_varend;
	char	*_start;
	char	*_end;

	memset(_varname, 0, sizeof(_varname));
	memset(_result, 0, sizeof(_result));
	_start = (char *) t;
	_end = strchr(_start, '$');
	if (_end) {
		while (_end) {
			if (_end - _start) {
				strncat(_result, _start, _end - _start);
			}
			_start = _end + 1;
			if (isalpha(*_start)) {
				_varend = _start;
				while (isalnum(*_varend)) {
					_varend++;
				}
				strncpy(_varname, _start, _varend - _start);
				if (getenv(_varname)) {
					strcat(_result, getenv(_varname));
				}
				_start = _varend;
			}
			else if (
				*_start == '?' ||
				*_start == '$'
			) {
				char	_s[128];
			
				memset(_s, 0, sizeof(_s));
				if (*_start == '?') {
					sprintf(_s, "%i", sh->exit_stat);
				}
				else if (*_start == '$') {	
					sprintf(_s, "%i", sh->pid);
				}
				strcat(_result, _s);
				_start++;
			}
			else {
				strcat(_result, "$");
			}
			_end = strchr(_start, '$');
		}
		strcat(_result, _start);
		return (strdup(_result));
	}
	_end = strchr(_start, '~');
	if (_end) {
		while (_end) {
			if (_end - _start) {
				strncat(_result, _start, _end - _start);
			}
			strcat(_result, getenv("HOME"));
			_start = _end + 1;
			_end = strchr(_start, '$');
		}
		strcat(_result, _start);
		return (strdup(_result));
	}
	return ((char *) t);
}

/*	Source:
 *	- https://lindevs.com/get-linux-distribution-name-using-cpp
 * */
char	*sh_distro(void) {
	static char	_distro[64];
	char		*_line;
	regex_t		_regex;
	regmatch_t	_regmatch[2];
	int			_regi;
	int			_fd;

	memset(_distro, 0, sizeof(_distro));
	_fd = open("/etc/os-release", O_RDONLY);
	if (_fd == -1) {
		perror("open");
		return (0);
	}
	_regi = regcomp(&_regex, "^NAME=\"(.*?)\"$", REG_EXTENDED);
	if (_regi) {
		perror("regcomp");
		close(_fd);
		return (0);
	}
	_line = sh_getline(_fd);
	while (_line) {
		if (!regexec(&_regex, _line, 2, _regmatch, 0)) {
			strncpy(_distro, _line + _regmatch[1].rm_so, _regmatch[1].rm_eo - _regmatch[1].rm_so);
			for (size_t i = 0; i < strlen(_distro); i++) {
				_distro[i] = tolower(_distro[i]);
			}
			free(_line);
			break;
		}
		free(_line);
		_line = sh_getline(_fd);
	}
	regfree(&_regex);
	close(_fd);
	return (_distro);
}
