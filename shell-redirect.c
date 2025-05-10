#include "shell.h"

char	**sh_handle_redirect(char **av) {
	sh_handle_input_redir(av);
	sh_handle_output_redir(av);
	if (!strcmp(*av, "<") || !strcmp(*av, ">") || !strcmp(*av, ">>")) {
		av += 2;
	}
	return (av);
}

void	sh_handle_input_redir(char **av) {
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

void	sh_handle_output_redir(char **av) {
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

void	sh_reset_redirect(struct s_shell *sh) {
	if (!sh) {
		return;
	}
	
	dup2(sh->fd_stdin, 0);
	dup2(sh->fd_stdout, 1);
}
