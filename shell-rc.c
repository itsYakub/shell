#include "shell.h"

/*	rcfile processing:
 *	1. Open rcfile.
 *	1.1. If rcfile doesn't exist, create one and exit.
 *	2. Process rcfile line by line
 *	2.1. If the first character is '#': skip
 *	2.2. All the rest of the lines must be processed as a regular command in the command-line
 * */

int		sh_rc(t_sh *sh, t_path fp) {
	bool	_read_dotfiles;

	_read_dotfiles = sh->settings[_shell_setting_read_dotfiles_];
	if (!_read_dotfiles) {
		return (1);
	}

	/* ...Opening dotfile... */
	sh->fd_curin = open(fp, O_RDONLY);
	if (sh->fd_curin == -1) {
		sh->fd_curin = open(fp, O_RDWR | O_CREAT, 0664);
		if (sh->fd_curin == -1) {
			perror("open");
			return (0);
		}
		return (1);
	}
	
	/* ...Processing dotfile... */
	sh->settings[_shell_setting_silent_] = true;
	sh_loop(sh);
	sh->settings[_shell_setting_silent_] = false;
	
	/* ...finish */
	close(sh->fd_curin); sh->fd_curin = -1;
	return (1);
}
