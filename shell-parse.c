#include "shell.h"

#if !defined (SH_TOKEN_CAPACITY)
# define SH_TOKEN_CAPACITY 64
#endif

static char	*__sh_extract(const char *line, size_t *);
static char	**__sh_extract_aliases(t_sh *, char **, size_t);

char	**sh_parse(const char *line, t_sh *sh) {
	char	**_tok;
	size_t	_tokcap;
	
	_tok = sh_lnsplt(line, &_tokcap);
	if (!_tok) {
		return (0);
	}
	if (!sh_parse_err(_tok)) {
		sh_free2d((void **) _tok);
		return (0);
	}
	_tok = __sh_extract_aliases(sh, _tok, _tokcap);
	return (_tok);
}

char	**sh_lnsplt(const char *line, size_t *c) {
	char	**_arr;
	size_t	_tok_cnt;
	size_t	_tok_cap;
	size_t	_tok_off;

	_tok_cnt = 0;
	_tok_cap = SH_TOKEN_CAPACITY;
	_arr = calloc(_tok_cap + 1, sizeof(char *));
	if (!_arr)
		return (0);
	while (*line) {
		/* if we exceed the size of the token vector, resize it by 16 elements (+1 for null entry) */
		if (_tok_cnt >= _tok_cap - 1) {
			_tok_cap += SH_TOKEN_CAPACITY;
			_arr = realloc(_arr, (_tok_cap + 1) * sizeof(char *));
			if (!_arr) {
				sh_free2d((void **) _arr);
				return (0);
			}
			for (size_t j = _tok_cnt; j < _tok_cap; j++) {
				_arr[j] = 0;
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
	*c = _tok_cap;
	return (_arr);
}

bool	sh_parse_err(char **t) {
	while (*t) {
		/* check if redirection has a valid direction after the operator */
		if (!strcmp(*t, "<") || !strcmp(*t, ">") || !strcmp(*t, ">>")) {
			if (!*(t + 1) || !sh_iskeyword(*(t + 1))) {
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

/*	ll -> 'ls -la'
 *	---
 *	ll | cat -e ; ll
 *	ls -la | cat -e ; ll
 *	ls -la | cat -e ; ls -la
 *	---
 *	1. Iterate over the token list
 *	1.1. For each token in the list, check if it has a valid alias assigned to it
 *	2. If we find an alias:
 *	2.1. Tokenize the alias
 *	2.2. Reallocate the token array to match the new size
 *	2.3. Copy all the tokens from the backward until we meet the alias
 *	2.4. Replace the alias token (and it's neighbours) with the new tokens
 *	2.5. Jump over the new tokens
 *	2.6. Go to step 1.
 * */

static char **__sh_extract_aliases(t_sh *sh, char **tok, size_t cap) {
	char	**_alias_tok;
	size_t	_alias_cnt;
	size_t	_tok_cnt;

	for (_tok_cnt = 0; tok[_tok_cnt]; _tok_cnt++) { }
	for (size_t i = 0; i < _tok_cnt; i++) {
		if (
			sh_alias_exist(sh, tok[i]) &&
			strcmp(tok[i > 0 ? i-1 : 0], "unalias") &&
			strcmp(tok[i > 0 ? i-1 : 0], "type")
		) {
			_alias_tok = sh_alias_extract(sh, tok[i], &_alias_cnt);
			if (!_alias_tok) {
				continue;
			}
			for (_alias_cnt = 0; _alias_tok[_alias_cnt]; _alias_cnt++) { }
			_tok_cnt += _alias_cnt;
			while (_tok_cnt >= cap) {
				cap += SH_TOKEN_CAPACITY;
				tok = realloc(tok, (cap + 1) * sizeof(char *));
				if (!tok) {
					sh_free2d((void **) tok);
					return (0);
				}
				for (size_t j = _tok_cnt; j < cap; j++) {
					tok[j] = 0;
				}
			}
			for (size_t j = _tok_cnt; tok[j] != tok[i + 1]; j--) {
				tok[j] = tok[j - _alias_cnt + 1];
			}
			free(tok[i]);
			for (size_t j = 0; j < _alias_cnt; j++) {
				tok[i++] = strdup(_alias_tok[j]);
			}
			sh_free2d((void **) _alias_tok); _alias_tok = 0;
			_alias_cnt = 0;
		}
	}
	return (tok);
}
