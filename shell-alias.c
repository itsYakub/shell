#include "shell.h"

char	**sh_alias_extract(t_sh *sh, const char *key, size_t *siz) {
	char	**_tokens;
	char	*_value;

	_value = sh_kvll_value(sh->aliases, (void *) key);
	if (!_value) {
		return (0);
	}
	_tokens = sh_lnsplt(_value, siz);
	return (_tokens);
}

bool	sh_alias_exist(t_sh *sh, const char *key) {
	t_kvll	*_head;

	_head = sh->aliases;
	while (_head) {
		if (key && !strcmp(key, _head->key)) {
			return (true);
		}
		_head = _head->next;
	}
	return (false);
}

int	sh_alias_export(t_sh *sh, const char *key, const char *value) {
	t_kvll	*_tmp;

	_tmp = sh->aliases;
	while (_tmp) {
		if (!strcmp(key, _tmp->key)) {
			free(_tmp->value);
			_tmp->value = (void *) value;
		}
		_tmp = _tmp->next;
	}
	sh_kvll_push(&sh->aliases, sh_kvll((void *) key, (void *) value));
	return (1);
}

int	sh_alias_clear(t_sh *sh) {
	sh_kvll_clear(sh->aliases);
	return (1);
}
