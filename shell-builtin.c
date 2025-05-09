#include "shell.h"

static void	__sh_bltin_type_loc(const char *);

bool	sh_isbltin(const char *s) {
	return (
		!strcmp(s, "exit") ||
		!strcmp(s, "type") ||
		!strcmp(s, "cd")
	);
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
		if (*_path)
			fprintf(stdout, "%s\n", _path);
		else
			fprintf(stdout, "(none)\n");
	}
}
