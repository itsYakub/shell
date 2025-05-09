#include "shell.h"

static int		__sh_exec(char **);

static size_t	__sh_pipe_count(char **);

static char		**__sh_next_pipe(char **);
static char		**__sh_next_command(char **);

static char		**__sh_handle_redirect(char **);
static void		__sh_handle_input_redir(char **);
static void		__sh_handle_output_redir(char **);

int	sh_execute(struct s_shell *sh) {
	char	**_cmd;
	int		_pipefd[2];
	int		_savestd[2];

	_cmd = sh->tokens;
	_savestd[0] = dup(0);
	_savestd[1] = dup(1);

	/*	TODO:
	 *	- Fix the problem with FDs in the pipeline
	 *	- Get back built-ins to work
	 * */

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
			if (!strcmp(*_cmd, "exit")) {
				dup2(0, _savestd[0]); close(_savestd[0]);
				dup2(1, _savestd[1]); close(_savestd[1]);
				sh_free(sh);
				exit(0);
			}
			else if (!strcmp(*_cmd, "cd")) {
				if (chdir(*(_cmd + 1)) == -1) {
					perror("cd");
				}
			}
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
				if (!fork()) {
					dup2(_pipefd[1], 1);
					close(_pipefd[0]);
					__sh_exec(_cmd);
				}
				dup2(_pipefd[0], 0);
				close(_pipefd[1]);
			}
			_cmd = __sh_handle_redirect(_cmd);
			if (!fork()) {
				__sh_exec(_cmd);
			}
			else {
				while (waitpid(-1, &sh->exit_stat, WUNTRACED) >= 0) { }
				dup2(_savestd[0], 0);
				dup2(_savestd[1], 1);
			}
		}

		/* After every command, get the standard file - descriptors back to normal */
		dup2(_savestd[0], 0);
		dup2(_savestd[1], 1);
		
		_cmd = __sh_next_command(_cmd);
	}

	/* clean-up */
	dup2(0, _savestd[0]); close(_savestd[0]);
	dup2(1, _savestd[1]); close(_savestd[1]);
	return (1);
}

static int	__sh_exec(char **av) {
	char	**_avcp;

	_avcp = av;
	while (
		*_avcp && (
			strcmp(*_avcp, ";") &&
			strcmp(*_avcp, "&&") &&
			strcmp(*_avcp, "|") &&
			strcmp(*_avcp, "<") &&
			strcmp(*_avcp, ">") &&
			strcmp(*_avcp, ">>")
		)
	) {
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

static char		**__sh_handle_redirect(char **av) {
	__sh_handle_input_redir(av);
	__sh_handle_output_redir(av);
	if (!strcmp(*av, "<") || !strcmp(*av, ">") || !strcmp(*av, ">>")) {
		av += 2;
	}
	return (av);
}

static void	__sh_handle_input_redir(char **av) {
	int	_fd;

	_fd = -1;
	while (*av && ((strcmp(*av, ";") && strcmp(*av, "&&") && strcmp(*av, "|")))) {
		if (*av && (!strcmp(*av, "<"))) {
			av++;
			_fd = open(*av, O_RDONLY, 0644);
			if (_fd == -1) {
				perror("shell");
				return;
			}
			dup2(_fd, 0);
			close(_fd);
		}
		av++;
	}
}

static void	__sh_handle_output_redir(char **av) {
	int	_fd;

	_fd = -1;
	while (*av && ((strcmp(*av, ";") && strcmp(*av, "&&") && strcmp(*av, "|")))) {
		if (*av && (!strcmp(*av, ">"))) {
			av++;
			_fd = open(*av, O_TRUNC | O_CREAT | O_RDWR, 0644);
			dup2(_fd, 1);
			close(_fd);
		}
		else if (*av && (!strcmp(*av, ">>"))) {
			av++;
			_fd = open(*av, O_CREAT | O_RDWR, 0644);
			dup2(_fd, 1);
			close(_fd);
		}
		av++;
	}
}
