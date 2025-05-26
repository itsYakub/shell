#include "shell.h"

static void	__sh_enable_ctrlc(int);
static void	__sh_disable_ctrlc(int);
static void	__sh_setup_env(void);

int main(void) {
	t_sh	_sh;

	sh_init(&_sh);
	while (1) {
		signal(SIGINT, __sh_enable_ctrlc);
		_sh.input = readline("$ ");
		fflush(stdin);
		signal(SIGINT, __sh_disable_ctrlc);
		if (_sh.input) {
			add_history(_sh.input);
			_sh.tokens = sh_parse(_sh.input, &_sh);
			if (_sh.tokens) {
				sh_execute(&_sh);
			}
		}
		sh_free(&_sh);
	}
	sh_quit(&_sh);
	exit(0);
}

int	sh_init(t_sh *sh) {
	if (!sh) {
		return (0);
	}
	/* Shell object setup */
	memset(sh, 0, sizeof(struct s_shell));
	sh->fd_stdin = dup(0);
	sh->fd_stdout = dup(0);
	sh->pid = getpid();

	/* Environment setup */
	__sh_setup_env();

	/* Reading rcfile */
	if (!sh_rc(sh)) {
		return (0);
	}
	return (1);
}

int	sh_quit(t_sh *sh) {	
	if (!sh) {
		return (0);
	}
	close(sh->fd_pipe[0]);
	close(sh->fd_pipe[1]);
	dup2(sh->fd_stdin, 0); close(sh->fd_stdin);
	dup2(sh->fd_stdout, 1); close(sh->fd_stdout);
	sh_free(sh);
	sh_alias_clear(sh);
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

static void	__sh_setup_env(void) {
	char	_cwd[PATH_MAX];
	char	*_shlvlstr;
	int		_shlvl;

	/* Setting - up $SHLVL */
	_shlvlstr = getenv("SHLVL");
	if (!_shlvlstr) {
		_shlvl = 1;
	}
	else {
		_shlvl = atoi(_shlvlstr) + 1;
	}
	sh_exporti("SHLVL", _shlvl);

	/* Setting - up $PWD */	
	if (getcwd(_cwd, PATH_MAX)) {
		sh_export("OLDPWD", _cwd);
		sh_export("PWD", _cwd);
	}
}
