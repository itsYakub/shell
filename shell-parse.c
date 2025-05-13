#include "shell.h"

static char	*__sh_extract(const char *line, size_t *);

char	**sh_parse(struct s_shell *sh) {
	char	**_tok;
	
	_tok = sh_lnsplt(sh->input);
	if (!sh_parse_err(_tok)) {
		sh_free2d((void **) _tok);
		return (0);
	}
	return (_tok);
}

char	**sh_lnsplt(const char *line) {
	char	**_arr;
	size_t	_tok_cnt;
	size_t	_tok_cap;
	size_t	_tok_off;

	_tok_cnt = 0;
	_tok_cap = 16;
	_arr = calloc(_tok_cap + 1, sizeof(char *));
	if (!_arr)
		return (0);
	while (*line) {
		/* if we exceed the size of the token vector, resize it by 16 elements (+1 for null entry) */
		if (_tok_cnt >= _tok_cap - 1) {
			_tok_cap += 16;
			_arr = realloc(_arr, (_tok_cap + 1) * sizeof(char *));
			if (!_arr) {
				sh_free2d((void **) _arr);
				return (0);
			}
		}
		_arr[_tok_cnt] = __sh_extract(line, &_tok_off);
		if (!_arr[_tok_cnt]) {
			sh_free2d((void **) _arr);
			return (0);
		}
		_tok_cnt++;
		line += _tok_off;
		while (*line && isspace(*line))
			line++;
	}
	return (_arr);
}

bool	sh_parse_err(char **t) {
	while (*t) {
		/* check if redirection has a valid direction after the operator */
		if (!strcmp(*t, "<") || !strcmp(*t, ">") || !strcmp(*t, ">>")) {
			if (*(t + 1) || !sh_iskeyword(*(t + 1))) {
				return (!fprintf(stderr, "shell: parse error: %s\n", *t));
			}
		}
		/* check if pipe is correctly used (isn't the last token and the pipe has keywords) */
		else if (!strcmp(*t, "|")) {
			if (!*(t + 1) || !strcmp(*(t + 1), "|")) {
				return (!fprintf(stderr, "shell: parse error: %s\n", *t));
			}
		}
		/* check if next-command is correctly used */
		else if (!strcmp(*t, "&&") || !strcmp(*t, ";")) {
			if (!*(t + 1)) {
				return (!fprintf(stderr, "shell: parse error: %s\n", *t));
			}
		}
		t++;
	}
	return (true);
}

static char	*__sh_extract(const char *line, size_t *tok_off) {
	char	*_lcpy;
	char	*_str;

	*tok_off = 0;
	_lcpy = (char *) line;
	/* if we encounter a space character */
	while (*_lcpy && isspace(*_lcpy)) {
		_lcpy++;
	}
	line = _lcpy;
	if (!*line)
		return (0);
	/* if we encounter a "&&" operator */
	if (
		!strncmp(line, "&&", 2)
	) {
		_lcpy += 2;
	}
	/* if we encounter a "|" or ";" operators */
	else if (
		*line == '|' ||
		*line == ';'
	) {
		_lcpy++;
	}
	else if (
		*line == '\"'
	) {
		_lcpy = (char *) ++line;
		while (*_lcpy && *_lcpy != '\"') {
			_lcpy++;
		}
		if (!*_lcpy) {
			fprintf(stderr, "shell: undisclosed double-quote\n");
			return (0);
		}
		*tok_off = 2;
	}
	else if (
		*line == '\''
	) {
		_lcpy = (char *) ++line;
		while (*_lcpy && *_lcpy != '\'') {
			_lcpy++;
		}
		if (!*_lcpy) {
			fprintf(stderr, "shell: undisclosed single-quote\n");
			return (0);
		}
		*tok_off = 2;
	}
	else {
		while (
			*_lcpy &&
			!isspace(*_lcpy) &&
			*_lcpy != '|' &&
			*_lcpy != ';' &&
			*_lcpy != '\"' &&
			*_lcpy != '\'' &&
			strncmp(line, "&&", 2)
		) {
			_lcpy++;
		}
	}
	_str = strndup(line, _lcpy - line);
	if (!_str)
		return (0);
	*tok_off += strlen(_str);
	return (_str);
}
