#include "shell.h"

t_kvll	*sh_kvll(void *key, void *value) {
	t_kvll	*_ll;

	_ll = malloc(sizeof(t_kvll));
	if (!_ll) {
		return (0);
	}
	_ll->key = key;
	_ll->value = value;
	_ll->next = 0;
	return (_ll);
}

t_kvll	*sh_kvll_get(t_kvll *ll, void *key) {
	t_kvll	*_head;

	_head = ll;
	while (_head) {
		if (!strcmp(key, _head->key)) {
			return (_head);
		}
		_head = _head->next;
	}
	return (0);
}

t_kvll	*sh_kvll_last(t_kvll *ll) {
	while (ll->next) {
		ll = ll->next;
	}
	return (ll);
}

void	*sh_kvll_value(t_kvll *ll, void *key) {
	return (sh_kvll_get(ll, key)->value);
}

size_t	sh_kvll_size(t_kvll *ll) {
	size_t	_result;
	
	for (_result = 0; ll; _result++)
		ll = ll->next;
	return (_result);
}

int	sh_kvll_push(t_kvll **ll, t_kvll *n) {
	t_kvll	*_head;

	if (!n) {
		return (0);
	}
	else if (!*ll) {
		*ll = n;
		return (1);
	}
	_head = *ll;
	while (_head->next) {
		_head = _head->next;
	}
	_head->next = n;
	return (1);
}

int	sh_kvll_pop(t_kvll **ll, void *key) {
	t_kvll	*_head;
	t_kvll	*_tmp;

	if (!ll)
		return (0);
	if (sh_kvll_get(*ll, key) == *ll)
		return (sh_kvll_pop_front(ll));
	if (sh_kvll_last(*ll) == *ll)
		return (sh_kvll_pop_back(ll));
	_head = sh_kvll_get(*ll, key);
	_tmp = _head->next->next;
	free(_head->next);
	_head->next = _tmp;
	return (1);
}

int	sh_kvll_pop_front(t_kvll **ll) {
	t_kvll	*_tmp;

	if (!ll)
		return (0);
	_tmp = *ll;
	*ll = (*ll)->next;
	free(_tmp->key);
	free(_tmp->value);
	free(_tmp);
	return (1);
}

int	sh_kvll_pop_back(t_kvll **ll) {
	size_t	_lsiz;
	t_kvll	*_last;

	if (!ll)
		return (0);
	_lsiz = sh_kvll_size(*ll);
	while (_lsiz--) {
		_last = _last->next;
	}
	if (!_last)
		return (0);
	free(_last->key);
	free(_last->value);
	free(_last);
	return (1);
}

int	sh_kvll_clear(t_kvll *head) {
	t_kvll	*_tmp;

	while (head) {
		_tmp = head;
		head = head->next;
		sh_kvll_free(_tmp);
	}
	return (1);
}

int	sh_kvll_free(t_kvll *head) {
	free(head->key);
	free(head->value);
	free(head);
	return (1);
}
