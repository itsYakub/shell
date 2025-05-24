#include "shell.h"

static char	*__sh_expand_var(struct s_shell *, const char *);

char	**sh_expand(struct s_shell *sh, char **cmd) {
	char	**_cmd;

	_cmd = cmd;
	while (*_cmd && !sh_isdelim(*_cmd)) {
		*_cmd = __sh_expand_var(sh, *_cmd);
		_cmd++;
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

static char	*__sh_expand_var(struct s_shell *sh, const char *t) {
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
	free((void *) t);
	return (strdup(res));
}
