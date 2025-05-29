#include "shell.h"

static void	__sh_enable_ctrlc(int);
static void	__sh_disable_ctrlc(int);

int main(void) {
	t_sh	_sh;

	if (!sh_init(&_sh)) {
		return (1);
	}
	if (!sh_loop(&_sh, false)) {
		return (1);
	}
	if (!sh_quit(&_sh)) {
		return (1);
	}
	exit(0);
}

int	sh_init(t_sh *sh) {
	if (!sh) {
		return (0);
	}
	/* Shell object setup */
	if (!sh_init_struct(sh)) {
		return (0);
	}

	/* Environment setup */
	if (!sh_init_env()) {
		return (0);
	}

	/* Reading rcfile */
	if (!sh_rc(sh, sh_rc_local())) {
		return (0);
	}
	return (1);
}

int	sh_init_struct(t_sh *sh) {
	memset(sh, 0, sizeof(struct s_shell));
	sh->fd_stdin = dup(0);
	if (sh->fd_stdin == -1) {
		perror("dup2");
		return (0);
	}
	sh->fd_stdout = dup(1);
	if (sh->fd_stdout == -1) {
		perror("dup2");
		return (0);
	}
	sh->fd_null = open("/dev/null", O_WRONLY);
	if (sh->fd_null == -1) {
		perror("open");
		return (0);
	}	
	sh->pid = getpid();
	return (1);
}

int	sh_init_env(void) {
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
	return (1);
}

int	sh_loop(t_sh *sh, bool silent) {
	while (1) {
		signal(SIGINT, __sh_enable_ctrlc);
		if (silent) {
			dup2(sh->fd_null, 1);
			sh->input = sh_getline(sh->fd_curin);
			dup2(sh->fd_stdout, 1);
		}
		else {
			sh->input = readline(sh_statusline(sh));
		}
		signal(SIGINT, __sh_disable_ctrlc);
		if (sh->input) {
			if (strcmp(sh->input, "")) {
				if (!silent) {
					add_history(sh->input);
				}
				sh->tokens = sh_parse(sh->input, sh);
				if (sh->tokens && *sh->tokens[0] != '#') {
					sh_execute(sh);
				}
			}
			sh_free(sh);
		}
		else {
			if (silent) {
				break;
			}
		}
	}
	return (1);
}

int	sh_quit(t_sh *sh) {	
	if (!sh) {
		return (0);
	}
	sh_free(sh);
	sh_close_fds(sh);
	sh_alias_clear(sh);
	free(sh->statusline);
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
