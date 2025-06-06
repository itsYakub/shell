#include "shell.h"

static void	__sh_enable_ctrlc(int);
static void	__sh_disable_ctrlc(int);

int main(int ac, char **av) {
	t_sh	_sh;

	if (!sh_init(&_sh, ac, av)) {
		return (1);
	}
	if (!sh_loop(&_sh)) {
		return (1);
	}
	if (!sh_quit(&_sh)) {
		return (1);
	}
	exit(0);
}

int	sh_init(t_sh *sh, int ac, char **av) {
	t_path	_path_home;

	memset(sh, 0, sizeof(struct s_shell));
	sh_init_defopt(sh);
	sh_getopt(sh, ac, av);
	sh_init_struct(sh);
	sh_init_env();

	/* $HOME dotfiles */
	memset(_path_home, 0, PATH_MAX);
	strcat(_path_home, getenv("HOME"));
	strcat(_path_home, "/.shrc");
	sh_rc(sh, _path_home);
	return (1);
}

int	sh_init_defopt(t_sh *sh) {
	/* Default: run interactive mode */
	sh->settings[_shell_setting_mode_] = 1;
	/* Default: read dotfiles */
	sh->settings[_shell_setting_read_dotfiles_] = 1;
	/* Default: print prompts to the stdout */
	sh->settings[_shell_setting_silent_] = 0;
	return (1);
}

int	sh_init_struct(t_sh *sh) {
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
	sh->distro = sh_distro();
	return (1);
}

int	sh_init_env(void) {
	t_path	_cwd;
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

int	sh_loop(t_sh *sh) {
	bool	_silent;
	int		_mode;

	_mode = sh->settings[_shell_setting_mode_];
	_silent = sh->settings[_shell_setting_silent_];
	do {
		signal(SIGINT, __sh_enable_ctrlc);
		if (_silent) {
			dup2(sh->fd_null, 1);
			sh->input = sh_getline(sh->fd_curin);
			dup2(sh->fd_stdout, 1);
		}
		else {
			sh->input = readline(sh_statusline(sh));
		}
		signal(SIGINT, __sh_disable_ctrlc);
		if (sh->input) {
			if (!sh_handle_input(sh)) {
				break;
			}
		}
		else {
			if (_silent) {
				break;
			}
		}
	} while (_mode);
	return (1);
}

int	sh_handle_input(t_sh *sh) {
	bool	_silent;

	_silent = sh->settings[_shell_setting_silent_];
	if (strcmp(sh->input, "")) {
		if (!_silent) {
			add_history(sh->input);
		}
		sh->tokens = sh_parse(sh->input, sh);
		if (sh->tokens && *sh->tokens[0] != '#') {
			sh_execute(sh);
		}
	}
	sh_free_input(sh);
	return (1);
}

int	sh_quit(t_sh *sh) {	
	if (!sh) {
		return (0);
	}
	sh_free_input(sh);
	sh_close_fds(sh);
	sh_alias_clear(sh);
	if (sh->statusline) {
		free(sh->statusline); sh->statusline = 0;
	}
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
