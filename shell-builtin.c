#include "shell.h"

static void	__sh_bltin_type_loc(const char *);

bool	sh_isbltin(const char *s) {
	return (
		!strcmp(s, "exit")		||
		!strcmp(s, "cd")		||
		!strcmp(s, "export")	||
		!strcmp(s, "unset")		||
		!strcmp(s, "alias")		||
		!strcmp(s, "unalias")	||
		!strcmp(s, "true")		||
		!strcmp(s, "false")		||
		!strcmp(s, "statusline")
	);
}

bool	sh_isbltin_exec(const char *s) {
	return (
		!strcmp(s, "type")		||
		!strcmp(s, "pwd")		||
		!strcmp(s, "env")
	);
}

int	sh_bltin_exit(t_sh *sh, char **cmd) {
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
	t_path	_cwd;
	char	*_path;

	_path = 0;
	/* Path setup part */
	if (!*(cmd + 1)) {
		return (!printf("cd [ PATH ] ...\n"));
	}
	else if (!sh_iskeyword(*(cmd + 1))) {
		_path = getenv("HOME");
	}
	else {
		_path = *(cmd + 1);
	}

	/* Main builting execution:
	 *	- set $OLDPWD variable
	 *	- change directory
	 *	- set $PWD variable
	 * */
	if (getcwd(_cwd, PATH_MAX)) {
		sh_export("OLDPWD", _cwd);
	}
	if (chdir(_path) == -1) {
		perror("cd");
		return (0);
	}
	if (getcwd(_cwd, PATH_MAX)) {
		sh_export("PWD", _cwd);
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
			if (!sh_export(_key, "")) {
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
			if (!sh_export(_key, _value)) {
				perror("setenv");
				return (0);
			}
		}
		return (1);
	}
	return (0);
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

int	sh_bltin_alias(t_sh *sh, char **cmd) {
	t_kvll	*_head;
	
	cmd++;
	if (
		!*cmd ||
		!sh_iskeyword(*cmd) ||
		!*(cmd + 1) ||
		!sh_iskeyword(*(cmd + 1))
	) {
		_head = sh->aliases;
		while(_head) {
			printf("%s=\"%s\"\n", (const char *) _head->key, (const char *) _head->value);
			_head = _head->next;
		}
		return (1);
	}
	return (sh_alias_export(sh, strdup(*cmd), strdup(*(cmd + 1))));
}

int	sh_bltin_unalias(t_sh *sh, char **cmd) {
	cmd++;
	if (!*cmd || !sh_iskeyword(*cmd)) {
		return (!printf("unalias [ ALIAS-NAME ]\n"));
	}
	return (sh_kvll_pop(&sh->aliases, *cmd));
}

int	sh_bltin_true(t_sh *sh) {
	sh->exit_stat = 0;
	return (1);
}

int	sh_bltin_false(t_sh *sh) {
	sh->exit_stat = 1;
	return (1);
}

int	sh_bltin_statusline(t_sh *sh, char **cmd) {
	cmd++;
	if (!*cmd || !sh_iskeyword(*cmd)) {
		printf("\"%s\"\n", sh->statusline);
		return (1);
	}
	if (sh->statusline) {
		free(sh->statusline); sh->statusline = 0;
	}
	sh->statusline = strdup(*cmd);
	if (!sh->statusline) {
		perror("strdup");
		return (0);
	}
	return (1);
}

int	sh_bltin_type(t_sh *sh, char **cmd) {
	cmd++;
	if (!(*cmd))
		exit(1);
	if (sh_isbltin(*cmd) || sh_isbltin_exec(*cmd)) {
		printf("%s is a shell builtin\n", *cmd);
	}
	else if (sh_alias_exist(sh, *cmd)) {
		printf("%s is aliased to \'%s\'\n", *cmd, (const char *) sh_kvll_value(sh->aliases, *cmd));
	}
	else {
		printf("%s is ", *cmd);
		__sh_bltin_type_loc(*cmd);
	}
	exit(0);
}

int	sh_bltin_pwd(char **cmd) {
	t_path	_cwd;
	char	*_pwd;

	(void) cmd;
	_pwd = getenv("PWD");
	if (!_pwd) {
		if (!getcwd(_cwd, PATH_MAX)) {
			return (!printf("( error )\n"));
		}
		_pwd = _cwd;
	}
	printf("%s\n", _pwd);
	exit(0);
}

int	sh_bltin_env(char **cmd) {
	(void) cmd;
	for (size_t i = 0; environ[i]; i++) {
		printf("%s\n", environ[i]);
	}
	exit(0);
}

static void	__sh_bltin_type_loc(const char *util) {
	t_path	_path;
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
			_endp = strchr(_env, ':');
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
			printf("%s\n", _path);
		else
			printf("( null )\n");
	}
}
