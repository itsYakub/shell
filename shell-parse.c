#include "shell.h"

static char	*__sh_extract(const char *line);

char	**sh_lnsplt(const char *line) {
	char	**_arr;
	size_t	_tok_cnt;
	size_t	_tok_cap;

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
		_arr[_tok_cnt] = __sh_extract(line);
		if (!_arr[_tok_cnt]) {
			sh_free2d((void **) _arr);
			return (0);
		}
		_tok_cnt++;
		line += strlen(_arr[_tok_cnt - 1]);
		while (*line && isspace(*line))
			line++;
	}
	return (_arr);
}

static char	*__sh_extract(const char *line) {
	char	*_lcpy;
	char	*_str;

	_lcpy = (char *) line;
	/* if we encounter a space character */
	while (isspace(*_lcpy)) {
		_lcpy++;
	}
	line = _lcpy;
	/* if we encounter a "&&" operator */
	if (
		!strncmp(_lcpy, "&&", 2)
	) {
		_lcpy += 2;
	}
	/* if we encounter a "|" or ";" operators */
	else if (
		*_lcpy == '|' ||
		*_lcpy == ';'
	) {
		_lcpy++;
	}
	else {
		while (*_lcpy && !isspace(*_lcpy)) {
			_lcpy++;
		}
	}
	_str = strndup(line, _lcpy - line);
	if (!_str)
		return (0);
	return (_str);
}
