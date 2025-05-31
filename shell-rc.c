#include "shell.h"

/*	rcfile processing:
 *	1. Open rcfile.
 *	1.1. If rcfile doesn't exist, create one and exit.
 *	2. Process rcfile line by line
 *	2.1. If the first character is '#': skip
 *	2.2. All the rest of the lines must be processed as a regular command in the command-line
 * */

int		sh_rc(t_sh *sh, const char *fp) {
	t_sh	_sh;

	sh_init_struct(&_sh);
	/* ...Opening dotfile... */
	_sh.fd_curin = open(fp, O_RDONLY);
	if (_sh.fd_curin == -1) {
		_sh.fd_curin = open(fp, O_RDWR | O_CREAT, 0664);
		if (_sh.fd_curin == -1) {
			perror("open");
			return (0);
		}
		return (1);
	}
	
	/* ...Processing dotfile... */
	sh_loop(&_sh, true);
	
	/* ...finish */
	sh->aliases = _sh.aliases;
	if (_sh.statusline) {
		sh->statusline = strdup(_sh.statusline);
		free(_sh.statusline);
	}
	sh_free(&_sh);
	sh_close_fds(&_sh);
	return (1);
}
