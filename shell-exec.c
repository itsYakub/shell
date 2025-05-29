#include "shell.h"

static size_t	__sh_pipe_count(char **);

static char		**__sh_next_pipe(char **);
static char		**__sh_next_command(char **);

int	sh_execute(t_sh *sh) {
	char	**_cmd;

	_cmd = sh->tokens;
	while (*_cmd) {
		/* Skipping the special characters */
		if (!strcmp(*_cmd, ";") || !strcmp(*_cmd, "|") || !strcmp(*_cmd, "&&")) {
			_cmd++;
			continue;
		}

		/* "&&" operator */
		if (*_cmd && _cmd != sh->tokens) {
			if (!strcmp(*(_cmd - 1), "&&")) {
				/* By convention, every other value than 0 means "failure"
				 * "&&" executes another command only if the previous one finished successfully
				 * */
				if (sh->exit_stat) {
					_cmd = __sh_next_command(_cmd);
					continue;
				}
			}
		}

		/* Expand variables */
		_cmd = sh_expand(sh, _cmd);

		/* Command execution happens here */
		if (sh_isbltin(*_cmd)) {
			/* builtin: exit */
			if (!strcmp(*_cmd, "exit")) {
				sh_bltin_exit(sh, _cmd);
			}
			/* builtin: cd */
			else if (!strcmp(*_cmd, "cd")) {
				sh_bltin_cd(_cmd);
			}
			/* builtin: unset */
			else if (!strcmp(*_cmd, "unset")) {
				sh_bltin_unset(_cmd);
			}
			/* builtin: export */
			else if (!strcmp(*_cmd, "export")) {
				sh_bltin_export(_cmd);
			}
			/* builtin: alias */
			else if (!strcmp(*_cmd, "alias")) {
				sh_bltin_alias(sh, _cmd);
			}
			/* builtin: unalias */
			else if (!strcmp(*_cmd, "unalias")) {
				sh_bltin_unalias(sh, _cmd);
			}
			/* builtin: ture */
			else if (!strcmp(*_cmd, "true")) {
				sh_bltin_true(sh);
			}
			/* builtin: false */
			else if (!strcmp(*_cmd, "false")) {
				sh_bltin_false(sh);
			}
			/* builtin: statusline */
			else if (!strcmp(*_cmd, "statusline")) {
				sh_bltin_statusline(sh, _cmd);
			}
		}
		else {
			for (
				int i = 0, pipc = __sh_pipe_count(_cmd);
				i < pipc - 1;
				i++, _cmd = __sh_next_pipe(_cmd)
			) {
				if (pipe(sh->fd_pipe) == -1) {
					perror("pipe");
					break;
				}
				_cmd = sh_handle_redirect(_cmd);
				if (!fork()) {
					dup2(sh->fd_pipe[1], 1);
					close(sh->fd_pipe[0]);
					sh_exec(sh, _cmd);
				}
				sh_reset_redirect(sh);
				dup2(sh->fd_pipe[0], 0);
				close(sh->fd_pipe[1]);
			}
			_cmd = sh_handle_redirect(_cmd);
			if (!fork()) {
				sh_exec(sh, _cmd);
			}
			else {
				while (waitpid(-1, &sh->exit_stat, WUNTRACED) >= 0) { }
				sh_reset_redirect(sh);
			}
		}

		_cmd = __sh_next_command(_cmd);
	}

	return (1);
}

static size_t	__sh_pipe_count(char **av) {
	size_t	_cnt;
	
	_cnt = 1;
	while (*av && (strcmp(*av, ";") && strcmp(*av, "&&"))) {
		_cnt += !strcmp(*av, "|");
		av++;
	}
	return (_cnt);
}

static char	**__sh_next_pipe(char **av) {
	while (*av && ((strcmp(*av, ";") && strcmp(*av, "&&") && strcmp(*av, "|")))) {
		av++;
		if (*av && (!strcmp(*av, ";") || !strcmp(*av, "&&") || !strcmp(*av, "|"))) {
			av++;
			break;
		}
	}
	return (av);
}

static char	**__sh_next_command(char **av) {
	while (*av && ((strcmp(*av, ";") && strcmp(*av, "&&") && strcmp(*av, "|")))) {
		av++;
		if (*av && (!strcmp(*av, ";") || !strcmp(*av, "&&"))) {
			av++;
			break;
		}
	}
	return (av);
}
