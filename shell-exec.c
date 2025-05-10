#include "shell.h"

static int		__sh_exec(char **);

static size_t	__sh_pipe_count(char **);

static char		**__sh_next_pipe(char **);
static char		**__sh_next_command(char **);

int	sh_execute(struct s_shell *sh) {
	char	**_cmd;
	int		_pipefd[2];

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
			/* builtin: type */
			else if (!strcmp(*_cmd, "type")) {
				sh_bltin_type(_cmd);
			}
		}
		else {
			for (
				int i = 0, pipc = __sh_pipe_count(_cmd);
				i < pipc - 1;
				i++, _cmd = __sh_next_pipe(_cmd)
			) {
				pipe(_pipefd);
				_cmd = sh_handle_redirect(_cmd);
				if (!fork()) {
					dup2(_pipefd[1], 1);
					close(_pipefd[0]);
					__sh_exec(_cmd);
				}
				sh_reset_redirect(sh);
				dup2(_pipefd[0], 0);
				close(_pipefd[1]);
			}
			_cmd = sh_handle_redirect(_cmd);
			if (!fork()) {
				__sh_exec(_cmd);
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

static int	__sh_exec(char **av) {
	char	**_avcp;

	_avcp = av;
	while (*_avcp && sh_iskeyword(*_avcp)) {
		_avcp++;
	}
	*_avcp = 0;
	execvp(*av, av);
	perror("execvp");
	exit(1);
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
