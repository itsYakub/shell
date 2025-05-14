#include "shell.h"

static void	__sh_bltin_type_loc(const char *);
static int	__sh_bltin_export_push(const char *, const char *);

bool	sh_isbltin(const char *s) {
	return (
		!strcmp(s, "exit")		||
		!strcmp(s, "cd")		||
		!strcmp(s, "unset")		||
		!strcmp(s, "export")	||
		!strcmp(s, "true")		||
		!strcmp(s, "false")
	);
}

bool	sh_isbltin_exec(const char *s) {
	return (
		!strcmp(s, "type")		||
		!strcmp(s, "pwd")		||
		!strcmp(s, "env")
	);
}

int	sh_bltin_exit(struct s_shell *sh, char **cmd) {
	int	_exit;

	if (*(cmd + 1) && sh_iskeyword(*(cmd + 1))) {
		_exit = atoi(*(cmd + 1));
	}
	else {
		_exit = 0;
	}

	sh_quit(sh);
	exit(_exit);
}

int	sh_bltin_cd(char **cmd) {
	char	*path;

	path = 0;
	if (!*(cmd + 1)) {
		return (!fprintf(stderr, "cd [ PATH ] ...\n"));
	}
	else if (!sh_iskeyword(*(cmd + 1))) {
		path = getenv("HOME");
	}
	else {
		path = *(cmd + 1);
	}
	if (chdir(path) == -1) {
		perror("cd");
		return (0);
	}
	return (1);
}

int	sh_bltin_unset(char **cmd) {
	cmd++;
	if (!*cmd || !sh_iskeyword(*cmd)) {
		return (0);
	}
	while (*cmd && sh_iskeyword(*cmd)) {
		if (unsetenv(*cmd)) {
			perror("unsetenv");
			return (0);
		}
		cmd++;
	}
	return (1);
}

int	sh_bltin_export(char **cmd) {
	char	*_key;
	char	*_value;
	char	*_start;
	char	*_end;

	cmd++;
	if (!*cmd) {
		if (!fork()) {
			sh_bltin_env(cmd);
		}
		return (1);
	}
	if (!*(cmd + 1) || strcmp(*(cmd + 1), "|")) {
		_key = _value = 0;
		_start = _end = *cmd;
		while (*_end && *_end != '=')
			_end++;
		_key = (char *) calloc(_end - _start, sizeof(char));
		if (!_key) {
			perror("calloc");
			return (0);
		}
		_key = strncpy(_key, _start, _end - _start);
		if (!*_end) {
			if (!__sh_bltin_export_push(_key, "")) {
				perror("setenv");
				return (0);
			}
		}
		else {
			_start = ++_end;
			while (*_end)
				_end++;
			_value = (char *) calloc(_end - _start + 1, sizeof(char));
			if (!_value) {
				perror("calloc");
				free(_key);
				return (0);
			}
			_value = strncpy(_value, _start, _end - _start);
			if (!__sh_bltin_export_push(_key, _value)) {
				perror("setenv");
				return (0);
			}
		}
		return (1);
	}
	return (0);
}

int	sh_bltin_true(struct s_shell *sh) {
	sh->exit_stat = 0;
	return (1);
}

int	sh_bltin_false(struct s_shell *sh) {
	sh->exit_stat = 1;
	return (1);
}

int	sh_bltin_type(char **cmd) {
	cmd++;
	if (!(*cmd))
		exit(1);
	if (sh_isbltin(*cmd) || sh_isbltin_exec(*cmd)) {
		write(1, *cmd, strlen(*cmd));
		write(1, " is a shell builtin\n", 20);
	}
	else {
		write(1, *cmd, strlen(*cmd));
		write(1, " is ", 4);
		__sh_bltin_type_loc(*cmd);
	}
	exit(0);
}

int	sh_bltin_pwd(char **cmd) {
	char	*_pwd;

	(void) cmd;
	_pwd = getenv("PWD");
	if (!_pwd) {
		return (!write(1, "( error )\n", 10));
	}
	write(1, _pwd, strlen(_pwd));
	write(1, "\n", 1);
	exit(0);
}

int	sh_bltin_env(char **cmd) {
	(void) cmd;
	for (size_t i = 0; environ[i]; i++) {
		write(1, environ[i], strlen(environ[i]));
		write(1, "\n", 1);
	}
	exit(0);
}

static void	__sh_bltin_type_loc(const char *util) {
	char	_path[PATH_MAX];
	char	*_env;
	char	*_endp;

	memset(_path, 0, sizeof(_path));
	if (realpath(util, _path)) {
		fprintf(stdout, "%s\n", _path);
	}
	else {
		_env = getenv("PATH");
		if (!_env)
			return;
		while (*_env) {
			memset(_path, 0, sizeof(_path));
			_endp = strchrnul(_env, ':');
			strncpy(_path, _env, _endp - _env);
			strcat(_path, "/");
			strcat(_path, util);
			if (!access(_path, F_OK)) {
				break;
			}
			_env = _endp + 1;
		}
		/*	some ducktaping stuff here: 
		 *		path on linux always starts from root ('/')
		 *		if the path starts with other character, that means it's not a valid path
		 *		and the utility is "unknown"
		 * */
		if (*_path == '/')
			write(1, _path, strlen(_path));
		else
			write(1, "unknown", 7);
		write(1, "\n", 1);
	}
}

static int	__sh_bltin_export_push(const char *key, const char *value) {
	if (getenv(key)) {
		if (setenv(key, value, 1) == -1) {
			return (0);
		}
	}
	else {
		if (setenv(key, value, 0) == -1) {
			return (0);
		}
	}
	return (1);
}
