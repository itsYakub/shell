#include "shell.h"

static void	__sh_enable_ctrlc(int);
static void	__sh_disable_ctrlc(int);

int main(void) {
	struct s_shell	_sh;

	sh_init(&_sh);
	while (1) {
		signal(SIGINT, __sh_enable_ctrlc);
		_sh.input = readline("$ ");
		fflush(stdin);
		signal(SIGINT, __sh_disable_ctrlc);
		if (_sh.input) {
			add_history(_sh.input);
			_sh.tokens = sh_parse(&_sh);
			if (_sh.tokens) {
				sh_execute(&_sh);
			}
		}
		sh_free(&_sh);
	}
	sh_quit(&_sh);
	exit(0);
}

int	sh_init(struct s_shell *sh) {
	if (!sh) {
		return (0);
	}
	memset(sh, 0, sizeof(struct s_shell));
	sh->fd_stdin = dup(0);
	sh->fd_stdout = dup(0);
	sh->pid = getpid();
	return (1);
}

int	sh_quit(struct s_shell *sh) {	
	if (!sh) {
		return (0);
	}
	dup2(sh->fd_stdin, 0); close(sh->fd_stdin);
	dup2(sh->fd_stdout, 1); close(sh->fd_stdout);
	sh_free(sh);
	rl_clear_history();
	return (1);
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
