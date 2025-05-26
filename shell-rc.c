#include "shell.h"

/*	rcfile processing:
 *	1. Open rcfile.
 *	1.1. If rcfile doesn't exist, create one and exit.
 *	2. Process rcfile line by line
 *	2.1. If the first character is '#': skip
 *	2.2. All the rest of the lines must be processed as a regular command in the command-line
 * */

static char *__sh_getrcpath(void);
static char	*__sh_getline(int);
static int	__sh_rc_quit(t_sh *);

int		sh_rc(t_sh *sh) {
	struct s_shell	_sh;
	int				_fd;

	/* Opening rcfile... */
	_fd = open(__sh_getrcpath(), O_RDONLY);
	if (_fd == -1) {
		_fd = open(__sh_getrcpath(), O_RDWR | O_CREAT, 0664);
		if (_fd == -1) {
			perror("open");
			return (0);
		}
		return (1);
	}
	
	/* ...Processing rcfile... */
	memset(&_sh, 0, sizeof(struct s_shell));
	_sh.fd_stdin = dup(0);
	_sh.fd_stdout = dup(0);
	_sh.pid = getpid();
	while (1) {
		_sh.input = __sh_getline(_fd);
		if (_sh.input) {
			_sh.tokens = sh_parse(_sh.input, &_sh);
			if (_sh.tokens) {
				/* Comments - checking */
				if (*(_sh.tokens[0]) != '#') {
					sh_execute(&_sh);
				}
			}
		}
		else {
			break;
		}
		sh_free(&_sh);
	}

	/* ...finish */
	__sh_rc_quit(&_sh);
	sh->aliases = _sh.aliases;
	close(_fd);
	return (1);
}

static char *__sh_getrcpath(void) {
	static char	_path[PATH_MAX];

	strcat(_path, getenv("HOME"));
	strcat(_path, "/.shrc");
	return (_path);
}

static char	*__sh_getline(int fd) {
	size_t	_fstrlen;
	char	*_fstr;
	char	_c;

	_c = 0;
	_fstr = 0;
	_fstrlen = 0;
	while (read(fd, &_c, 1)) {
		if (!_c) {
			break;
		}
		else if (_c == '\n') {
			break;
		}
		_fstrlen++;
	}
	if (!_fstrlen)
		return (0);
	_fstr = (char *) calloc(_fstrlen + 1, sizeof(char));
	if (!_fstr) {
		perror("calloc");
		return (0);
	}
	if (lseek(fd, -(_fstrlen + 1), SEEK_CUR) == 1) {
		perror("lseek");
		free(_fstr);
		return (0);
	}
	if (read(fd, _fstr, _fstrlen) == -1) {
		perror("read");
		free(_fstr);
		return (0);
	}
	if (_c == '\n') {
		lseek(fd, 1, SEEK_CUR);
	}
	return (_fstr);
}

static int	__sh_rc_quit(t_sh *sh) {
	if (!sh) {
		return (0);
	}
	close(sh->fd_pipe[0]);
	close(sh->fd_pipe[1]);
	dup2(sh->fd_stdin, 0); close(sh->fd_stdin);
	dup2(sh->fd_stdout, 1); close(sh->fd_stdout);
	sh_free(sh);
	rl_clear_history();
	return (1);
}
