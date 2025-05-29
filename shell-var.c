#include "shell.h"

static char	*__sh_expstr(struct s_shell *, const char *);

	char	**sh_expand(t_sh *sh, char **cmd) {
	char	*_tmp;

	for (size_t i = 0; cmd[i] && !sh_isdelim(cmd[i]); i++) {
		_tmp = cmd[i];
		if (strcmp(cmd[i > 0 ? i-1 : 0], "statusline")) {
			cmd[i] = __sh_expstr(sh, cmd[i]);
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

int	sh_exporti(const char *key, int value) {
	char	_str[128];

	memset(_str, 0, sizeof(_str));
	snprintf(_str, 128, "%i", value);
	return (sh_export(key, _str));
}

static char	*__sh_expstr(struct s_shell *sh, const char *t) {
	char	res[1024];	/* res - result */
	char	vn[1024];	/* vn - variable name */
	char	*ve;		/* ve - variable end */
	char	*s;			/* s - start */
	char	*e;			/* e - end */

	memset(vn, 0, sizeof(vn));
	memset(res, 0, sizeof(res));
	s = (char *) t;
	e = strchr(s, '$');
	if (!e) {
		return ((char *) t);
	}
	while (e) {
		if (e - s) {
			strncat(res, s, e - s);
		}
		s = e + 1;
		if (isalpha(*s)) {
			ve = s;
			while (isalnum(*ve)) {
				ve++;
			}
			strncpy(vn, s, ve - s);
			if (getenv(vn)) {
				strcat(res, getenv(vn));
			}
			s = ve;
		}
		else if (
			*s == '?' ||
			*s == '$'
		) {
			char	_s[128];
		
			memset(_s, 0, sizeof(_s));
			if (*s == '?') {
				sprintf(_s, "%i", sh->exit_stat);
			}
			else if (*s == '$') {	
				sprintf(_s, "%i", sh->pid);
			}
			strcat(res, _s);
			s++;
		}
		else {
			strcat(res, "$");
		}
		e = strchr(s, '$');
	}
	strcat(res, s);
	return (strdup(res));
}
