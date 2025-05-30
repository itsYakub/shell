#include "shell.h"

const char	*__sh_shrink_homedir(char *);

/*	Formats:
 *	- %n = host-name
 *	- %d = desktop-name
 *	- %p = cwd
 *	- %t = time (hours:minutes:seconds)
 *	- %T = time (years:months:days)
 *	- %% = %
 * */
char	*sh_statusline(t_sh *sh) {
	static char		_statusline[1024];
	char			*_start;
	char			*_end;

	if (!sh->statusline) {
		return ("$ ");
	}
	memset(_statusline, 0, sizeof(_statusline));
	_start = _end = sh->statusline;
	while (_end) {
		_end = strchr(_start, '%');
		if (!_end) {
			return (strcat(_statusline, _start));
		}
		strncat(_statusline, _start, _end - _start);
		_end++;
		switch (*_end) {
			case ('n'): {
				char	*_login;

				_login = getlogin();
				if (!_login) {
					perror("gethostname");
					break;
				}
				strcat(_statusline, _login);
			} break;
			case ('d'): {
				struct utsname	_utsname;
				
				if (uname(&_utsname) == -1) {
					perror("uname");
					break;
				}
				strcat(_statusline, _utsname.sysname);
			} break;
			case ('p'): {
				char	_cwd[PATH_MAX];

				memset(_cwd, 0, sizeof(_cwd));
				if (!getcwd(_cwd, PATH_MAX)) {
					perror("getcwd");
					break;
				}
				strcat(_statusline, __sh_shrink_homedir(_cwd));
			} break;
			case ('t'): {
				struct tm	*_time;
				time_t		_traw;
				char		_tstr[32];
				
				memset(_tstr, 0, sizeof(_tstr));
				time(&_traw);
				_time = localtime(&_traw);
				sprintf(_tstr, "%02d:%02d:%02d", _time->tm_hour, _time->tm_min, _time->tm_sec);
				strcat(_statusline, _tstr);
			} break;
			case ('T'): {
				struct tm	*_time;
				time_t		_traw;
				char		_tstr[32];
				
				memset(_tstr, 0, sizeof(_tstr));
				time(&_traw);
				_time = localtime(&_traw);
				sprintf(_tstr, "%d:%02d:%02d", _time->tm_year + 1900 , _time->tm_mon + 1, _time->tm_mday);
				strcat(_statusline, _tstr);
			} break;
			case ('%'): {
				strcat(_statusline, "%");
			} break;
			case (0): {
				return (strcat(_statusline, "%"));
			}
			default: {
				strncat(_statusline, (_end - 1), 2);
			}
		}
		_start = _end + 1;
	}
	return (_statusline);
}

const char	*__sh_shrink_homedir(char *str) {
	const char	*_home;

	_home = getenv("HOME");
	for (size_t	i = 0; str[i]; i++) {
		if (!strncmp(&(str[i]), _home, strlen(_home))) {
			size_t	_mov_siz;
			char	*_start;
			char	*_end;

			_start = &(str[i+1]);
			_end = &(str[strlen(_home)]);
			_mov_siz = strlen(_end);
			if (_mov_siz) {
				memmove(_start, _end, _mov_siz);
				memset(_start + _mov_siz, 0, _mov_siz);
			}
			else {
				memset(_start, 0, _end - _start);
			}
			str[i] = '~';
		}
	}
	return (str);
}
