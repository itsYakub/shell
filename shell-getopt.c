#include "shell.h"

static int	__sh_helpmsg(void);

static struct option ___shell_longopt[] = {
    { "command",	required_argument,	NULL,	'c' },
    { "version",	no_argument,		NULL,	'v' },
    { "help",		no_argument,		NULL,	'h' },
    { 0,			0,					0,		0	}
};

int	sh_getopt(t_sh *sh, int ac, char **av) {
	char	_c;

	while ((_c = getopt_long(ac, av, "c:vh", ___shell_longopt, NULL)) != -1) {
		switch (_c) {
			case ('v') : { printf("%s\n", _shell_version_); exit(0); } break;
			case ('h') : { __sh_helpmsg(); exit(0); } break;
			case ('c') : {
				sh->settings[_shell_setting_mode_] = 0;
				sh->settings[_shell_setting_read_dotfiles_] = 0;
				sh->settings[_shell_setting_silent_] = 1;
				sh->input = strdup(optarg);
				sh_handle_input(sh);
				sh_quit(sh);
				exit (0);
			} break;
			default: { exit(0); } break;
		}
	}
	return (1);
}

static int	__sh_helpmsg(void) {
	printf("Shell help message *to be added*\n");
	return (1);
}
