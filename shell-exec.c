#include "shell.h"

static int		__sh_exec(char **, int);

static size_t	__sh_pipe_count(char **);

static char		**__sh_next_pipe(char **);
static char		**__sh_next_command(char **);

static void	__sh_next_input_redir(char **);
static void	__sh_next_output_redir(char **);

int	sh_execute(char **cmd) {
	char	**_cmd;
	int		_savestd[2];
	int		_pipefd[2];
	int		_tmpfd;
	int		_stat;

	_cmd = cmd;
	_savestd[0] = dup(0);
	_savestd[1] = dup(1);
	_tmpfd = dup(0);
	while (*_cmd) {
		/* Skipping the special characters */
		if (!strcmp(*_cmd, ";") || !strcmp(*_cmd, "|") || !strcmp(*_cmd, "&&")) {
			_cmd++;
			continue;
		}

		/* "&&" operator */
		if (*_cmd && _cmd != cmd) {
			if (!strcmp(*(_cmd - 1), "&&")) {
				/* By convention, every other value than 0 means "failure".
				 * "&&" executes another command only if the previous one finished successfully.
				 * */
				if (_stat) {
					_cmd = __sh_next_command(_cmd);
					continue;
				}
			}
		}

		/* Setting up redirection */
		__sh_next_input_redir(_cmd);
		__sh_next_output_redir(_cmd);

		/* If the command starts with redirection, skip it to the real command */
		if (!strcmp(*_cmd, "<") || !strcmp(*_cmd, ">") || !strcmp(*_cmd, ">>")) {
			_cmd += 2;
		}

		/* Command type: pipeline */
		if (__sh_pipe_count(_cmd) > 1) {
			if (!sh_isbltin(*_cmd)) {
				pipe(_pipefd);
				if (!fork()) {
					dup2(_pipefd[1], 1);
					close(_pipefd[0]);
					close(_pipefd[1]);
					__sh_exec(_cmd, _tmpfd);
				}
				else {
					close(_pipefd[1]);
					close(_tmpfd);
					_tmpfd = _pipefd[0];
				}
			}
			/* For now built-ins are only executed by the regular command and not by pipelines */
			else {
				fprintf(stderr, "warn: can't pipe built-in commands\n");
			}
			_cmd = __sh_next_pipe(_cmd);
		}
		/* Command type: regular */
		else {
			if (!sh_isbltin(*_cmd)) {
				if (!fork()) {
					__sh_exec(_cmd, _tmpfd);
				}
				else {
					close(_tmpfd);
					while (waitpid(-1, &_stat, WUNTRACED) != -1)
						continue;
					_tmpfd = dup(0);
				}
			}
			/* built-in commands */
			else {
				if (!strcmp(*_cmd, "exit")) {
					close(_tmpfd);
					dup2(0, _savestd[0]); close(_savestd[0]);
					dup2(1, _savestd[1]); close(_savestd[1]);
					sh_free2d((void **) _cmd);
					exit(0);
				}
				else if (!strcmp(*_cmd, "type")) {
					if (!sh_bltin_type(_cmd)) {
						break;
					}
				}
				else if (!strcmp(*_cmd, "cd")) {
					if (chdir(_cmd[1]) < 0) {
						perror("cd");
						break;
					}
				}
			}
			_cmd = __sh_next_command(_cmd);
		}
		/* After every command, get the standard file - descriptors back to normal */
		dup2(_savestd[0], 0);
		dup2(_savestd[1], 1);
	}

	/* clean-up */
	close(_tmpfd);
	dup2(0, _savestd[0]); close(_savestd[0]);
	dup2(1, _savestd[1]); close(_savestd[1]);
	return (1);
}

static int	__sh_exec(char **av, int fd) {
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
	dup2(fd, 0);
	close(fd);
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

static void	__sh_next_input_redir(char **av) {
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

static void	__sh_next_output_redir(char **av) {
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
