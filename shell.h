#if defined (__cplusplus)
# pragma once
#endif
#if !defined (_shell_h_)
# define _shell_h_
# if !defined (__cplusplus)
#  include <stdbool.h>
# endif
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/param.h>
# include <readline/readline.h>
# include <readline/history.h>

struct s_shell {
	char	**tokens;
	char	*input;
	int		exit_stat;
};

/* shell-exec.c */
int		sh_execute(struct s_shell *);

/* shell-parse.c */
char	**sh_lnsplt(const char *);

/* shell-utils.c */
void	sh_free(struct s_shell *);
void	sh_free2d(void **);
bool	sh_iskeyword(const char *);

/* shell-builtin.c */
bool	sh_isbltin(const char *);
int		sh_bltin_type(char **);

#endif
