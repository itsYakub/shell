#include "shell.h"

const char	*__sh_shrink_homedir(char *);

/*	Formats:
 *	- %n = host-name
 *	- %d = desktop-name
 *	- %p = cwd
 *	- %t = time (hours:minutes:seconds)
 *	- %T = time (years:months:days)
 *	- %% = %
 *	Workflow:
 *		This function iterates over the statusline format string stored in the struct 'sh'.
 *		Every time it finds a matching format (that starts with the '%') it starts processing the following
 *		character based on the specific rules. Every time it meets the proper format character, it concats the
 *		proper value to the statusline string.
 * */
char	*sh_statusline(t_sh *sh) {
	static char		_statusline[1024];
	char			*_str;

	if (!sh->statusline) {
		return ("$ ");
	}
	memset(_statusline, 0, sizeof(_statusline));
	_str = sh->statusline;
	for ( ; *_str; _str++) {
		if (*_str == '%') {
			_str++;
			switch (*_str) {
				case ('n'): {
					char	*_login;

					_login = getlogin();
					if (!_login) {
						perror("getlogin");
						break;
					}
					strcat(_statusline, _login);
				} break;
				case ('d'): {
					strcat(_statusline, sh->distro);
				} break;
				case ('p'): {
					t_path	_cwd;

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
					strncat(_statusline, (_str - 1), 2);
				}
			}
		}
		else {
			strncat(_statusline, _str, 1);
		}
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
