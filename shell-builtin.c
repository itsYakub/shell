#include "shell.h"

static void	__sh_bltin_type_loc(const char *);

bool	sh_isbltin(const char *s) {
	return (
		!strcmp(s, "exit")	||
		!strcmp(s, "cd")	||
		!strcmp(s, "type")	||
		!strcmp(s, "pwd")	||
		!strcmp(s, "env")	||
		!strcmp(s, "unset")	||
		!strcmp(s, "export")
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

int	sh_bltin_type(char **cmd) {
	cmd++;
	if (!(*cmd))
		return (0);
	if (sh_isbltin(*cmd))
		fprintf(stdout, "%s is a shell builtin\n", *cmd);
	else {
		fprintf(stdout, "%s is ", *cmd);
		__sh_bltin_type_loc(*cmd);
	}
	return (1);
}

int	sh_bltin_pwd(char **cmd) {
	char	*_pwd;

	(void) cmd;
	_pwd = getenv("PWD");
	if (!_pwd) {
		return (!write(0, "( error )\n", 10));
	}
	write(0, _pwd, strlen(_pwd));
	write(0, "\n", 1);
	return (1);
}

int	sh_bltin_env(char **cmd) {
	(void) cmd;
	for (size_t i = 0; environ[i]; i++) {
		write(0, environ[i], strlen(environ[i]));
		write(0, "\n", 1);
	}
	return (1);
}

int	sh_bltin_unset(char **cmd) {
	cmd++;
	if (!*cmd || !sh_iskeyword(*cmd)) {
		return (0);
	}
	if (unsetenv(*cmd)) {
		perror("unsetenv");
		return (0);
	}
	return (1);
}

int	sh_bltin_export(char **cmd) {
	(void) cmd;
	return (1);
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
			fprintf(stdout, "%s\n", _path);
		else
			fprintf(stdout, "unknown\n");
	}
}
