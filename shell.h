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
	int		fd_stdin;
	int		fd_stdout;
	int		fd_pipe[2];
	int		pid;
};

extern char	**environ;

/* shell.c */
int		sh_init(struct s_shell *);
int		sh_quit(struct s_shell *);

/* shell-exec.c */
int		sh_execute(struct s_shell *);

/* shell-parse.c */
char	**sh_parse(const char *);
char	**sh_lnsplt(const char *);
bool	sh_parse_err(char **);

/* shell-utils.c */
void	sh_free(struct s_shell *);
void	sh_free2d(void **);
bool	sh_iskeyword(const char *);
bool	sh_isdelim(const char *);
int		sh_exec(char **);

/* shell-builtin.c */
bool	sh_isbltin(const char *);
bool	sh_isbltin_exec(const char *);

int		sh_bltin_exit(struct s_shell *, char **);
int		sh_bltin_cd(char **);
int		sh_bltin_export(char **);
int		sh_bltin_unset(char **);
int		sh_bltin_true(struct s_shell *);
int		sh_bltin_false(struct s_shell *);

int		sh_bltin_type(char **);
int		sh_bltin_pwd(char **);
int		sh_bltin_env(char **);

/* shell-redirect.c */
char	**sh_handle_redirect(char **);
void	sh_handle_input_redir(char **);
void	sh_handle_output_redir(char **);
void	sh_reset_redirect(struct s_shell *);

/* shell-var.c */
char	**sh_expand(struct s_shell *, char **);
int		sh_export(const char *, const char *);
int		sh_exporti(const char *, int);

/* shell-rc.c */
int		sh_rc(void);

#endif
