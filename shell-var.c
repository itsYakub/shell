#include "shell.h"

char	**sh_expand(t_sh *sh, char **cmd) {
	char	*_tmp;

	for (size_t i = 0; cmd[i] && !sh_isdelim(cmd[i]); i++) {
		_tmp = cmd[i];
		if (strcmp(cmd[i > 0 ? i-1 : 0], "statusline")) {
			cmd[i] = sh_expstr(sh, cmd[i]);
			if (cmd[i] != _tmp) {
				free(_tmp);
			}
		}
	}
	return (cmd);
}

int	sh_export(const char *key, const char *value) {
	if (getenv(key)) {
		if (setenv(key, value, 1) == -1) {
			perror("setenv");
			return (0);
		}
	}
	else {
		if (setenv(key, value, 0) == -1) {
			perror("setenv");
			return (0);
		}
	}
	return (1);
}

int	sh_exporti(const char *key, int value) {
	char	_str[128];

	memset(_str, 0, sizeof(_str));
	snprintf(_str, 128, "%i", value);
	return (sh_export(key, _str));
}
