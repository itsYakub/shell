#include "shell.h"

static int		__sh_exec(char **, int);
static size_t	__sh_pipe_count(char **);
static char		**__sh_next_pipe(char **);
static char		**__sh_next_command(char **);

int	sh_execute(char **cmd) {
	int	_savestd[2];
	int	_pipefd[2];
	int	_tmpfd;
	int	_stat;

	_savestd[0] = dup(0);
	_savestd[1] = dup(1);
	_tmpfd = dup(0);
	while (*cmd) {
		if (!strcmp(*cmd, ";") || !strcmp(*cmd, "|") || !strcmp(*cmd, "&&")) {
			cmd++;
			continue;
		}
		if (__sh_pipe_count(cmd) > 1) {
			if (!sh_isbltin(*cmd)) {
				pipe(_pipefd);
				if (!fork()) {
					dup2(_pipefd[1], 1);
					close(_pipefd[0]);
					close(_pipefd[1]);
					__sh_exec(cmd, _tmpfd);
				}
				else {
					close(_pipefd[1]);
					close(_tmpfd);
					_tmpfd = _pipefd[0];
				}
			}
			else {
				fprintf(stderr, "warn: can't pipe built-in commands\n");
			}
			cmd = __sh_next_pipe(cmd);
		}
		else {
			if (!sh_isbltin(*cmd)) {
				if (!fork()) {
					__sh_exec(cmd, _tmpfd);
				}
				else {
					close(_tmpfd);
					while (waitpid(-1, &_stat, WUNTRACED) != -1)
						continue;
					_tmpfd = dup(0);
				}
			}
			/* TODO: Builtins */
			else {
				if (!strcmp(*cmd, "exit")) {
					close(_tmpfd);
					dup2(0, _savestd[0]); close(_savestd[0]);
					dup2(1, _savestd[1]); close(_savestd[1]);
					sh_free2d((void **) cmd);
					exit(0);
				}
				else if (!strcmp(*cmd, "type")) {
					if (!sh_bltin_type(cmd)) {
						break;
					}
				}
				else if (!strcmp(*cmd, "cd")) {
					if (chdir(cmd[1]) < 0) {
						perror("cd");
						break;
					}
				}
			}
			cmd = __sh_next_command(cmd);
		}
	}
	close(_tmpfd);
	dup2(0, _savestd[0]); close(_savestd[0]);
	dup2(1, _savestd[1]); close(_savestd[1]);
	return (1);
}

static int	__sh_exec(char **av, int fd) {
	char	**_avcp;

	_avcp = av;
	while (*_avcp && (strcmp(*_avcp, ";") && strcmp(*_avcp, "&&") && strcmp(*_avcp, "|")))
		_avcp++;
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
