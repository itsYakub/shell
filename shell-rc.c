#include "shell.h"

/*	rcfile processing:
 *	1. Open rcfile.
 *	1.1. If rcfile doesn't exist, create one and exit.
 *	2. Process rcfile line by line
 *	2.1. If the first character is '#': skip
 * */

static char *__sh_getrcpath(void);
static char	*__sh_getline(int);

int		sh_rc(struct s_shell *sh) {
	char	*_line;
	int		_fd;

	(void) sh;
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
	do {
		_line = __sh_getline(_fd);
		if (!_line) {
			break;
		}
		printf("%s\n", _line);
		free(_line);
	} while (_line);

	/* ...finish */
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
