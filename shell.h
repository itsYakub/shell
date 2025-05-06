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

/* shell-exec.c */
int		sh_execute(char **);

/* shell-parse.c */
char	**sh_lnsplt(const char *);

/* shell-utils.c */
void	sh_free2d(void **);
void	sh_clear_fbuf(FILE *);

/* shell-builtin.c */
bool	sh_isbltin(const char *);
int		sh_bltin_type(char **);

#endif
