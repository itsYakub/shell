#include "shell.h"

static void	__sh_enable_ctrlc(int);
static void	__sh_disable_ctrlc(int);

int main(void) {
	struct s_shell	_sh;

	while (1) {
		signal(SIGINT, __sh_enable_ctrlc);
		_sh.input = readline("$ ");
		fflush(stdin);
		signal(SIGINT, __sh_disable_ctrlc);
		if (_sh.input) {
			add_history(_sh.input);
			_sh.tokens = sh_lnsplt(_sh.input);
			if (_sh.tokens) {
				sh_execute(&_sh);
			}
		}
		sh_free(&_sh);
	}
	rl_clear_history();
	sh_free(&_sh);
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
