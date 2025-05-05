#include "shell.h"

static void	__sh_enable_ctrlc(int);
static void	__sh_disable_ctrlc(int);

int main(void) {
	char	**_tokens;
	char	*_line;

	_line = 0;
	_tokens = 0;
	while (1) {
		signal(SIGINT, __sh_enable_ctrlc);
		_line = readline("$ ");
		fflush(stdin);
		signal(SIGINT, __sh_disable_ctrlc);
		if (_line) {
			add_history(_line);
			_tokens = sh_lnsplt(_line);
			if (_tokens) {
				sh_execute(_tokens);
				sh_free2d((void **) _tokens); _tokens = 0;
			}
			free(_line); _line = 0;
		}
	}
	rl_clear_history();
	if (_line)
		free(_line);
	if (_tokens)
		sh_free2d((void **) _tokens);
	return (0);
}

static void	__sh_enable_ctrlc(int sig) {
	(void) sig;	
	printf("\n");
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

static void	__sh_disable_ctrlc(int sig) {
	(void) sig;
	printf("\n");
	rl_on_new_line();
	rl_replace_line("", 0);
}
