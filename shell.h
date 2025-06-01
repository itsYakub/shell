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
# include <regex.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/param.h>
# include <sys/utsname.h>
# include <readline/readline.h>
# include <readline/history.h>

struct s_kvll {
	void			*key;
	void			*value;
	struct s_kvll	*next;
};

typedef struct s_kvll	t_kvll;

struct s_shell {
	t_kvll	*aliases;
	char	**tokens;
	char	*input;
	char	*statusline;
	char	*distro;
	int		exit_stat;
	int		fd_curin;
	int		fd_stdin;
	int		fd_stdout;
	int		fd_null;
	int		pid;
	int		fd_pipe[2];
};

typedef struct s_shell	t_sh;

typedef char	t_path[PATH_MAX];

extern char	**environ;

/* shell.c */
int		sh_init(t_sh *);
int		sh_init_struct(t_sh *);
int		sh_init_env(void);
int		sh_loop(t_sh *, bool);
int		sh_quit(t_sh *);
int		sh_input(t_sh *);

/* shell-exec.c */
int		sh_execute(t_sh *);

/* shell-parse.c */
char	**sh_parse(const char *, t_sh *);
char	**sh_lnsplt(const char *, size_t *);
bool	sh_parse_err(char **);

/* shell-utils.c */
void	sh_free(t_sh *);
void	sh_free2d(void **);
void	sh_close_fds(t_sh *);
bool	sh_iskeyword(const char *);
bool	sh_isdelim(const char *);
int		sh_exec(t_sh *, char **);
char	*sh_getline(int);
char	*sh_strjoinc(char *, char);
char	*sh_expstr(struct s_shell *, const char *);
char	*sh_distro(void);

/* shell-builtin.c */
bool	sh_isbltin(const char *);
bool	sh_isbltin_exec(const char *);

int		sh_bltin_exit(t_sh *, char **);
int		sh_bltin_cd(char **);
int		sh_bltin_export(char **);
int		sh_bltin_unset(char **);
int		sh_bltin_alias(t_sh *, char **);
int		sh_bltin_unalias(t_sh *, char **);
int		sh_bltin_true(t_sh *);
int		sh_bltin_false(t_sh *);
int		sh_bltin_statusline(t_sh *, char **);

int		sh_bltin_type(t_sh *, char **);
int		sh_bltin_pwd(char **);
int		sh_bltin_env(char **);

/* shell-redirect.c */
char	**sh_handle_redirect(char **);
void	sh_handle_input_redir(char **);
void	sh_handle_output_redir(char **);
void	sh_reset_redirect(struct s_shell *);

/* shell-var.c */
char	**sh_expand(t_sh *, char **);
int		sh_export(const char *, const char *);
int		sh_exporti(const char *, int);

/* shell-rc.c */
int		sh_rc(t_sh *, t_path);

/* shell-kvll.c */
t_kvll	*sh_kvll(void *, void *);
t_kvll	*sh_kvll_dup(t_kvll *);
t_kvll	*sh_kvll_get(t_kvll *, void *);
t_kvll	*sh_kvll_last(t_kvll *);
void	*sh_kvll_value(t_kvll *, void *);
size_t	sh_kvll_size(t_kvll *);
int		sh_kvll_push(t_kvll **, t_kvll *);
int		sh_kvll_pop(t_kvll **, void *);
int		sh_kvll_pop_front(t_kvll **);
int		sh_kvll_pop_back(t_kvll **);
int		sh_kvll_clear(t_kvll *);
int		sh_kvll_free(t_kvll *);

/* shell-alias.c */
char	**sh_alias_extract(t_sh *, const char *, size_t *);
bool	sh_alias_exist(t_sh *, const char *);
int		sh_alias_export(t_sh *, const char *, const char *);
int		sh_alias_clear(t_sh *);

/* shell-statusline.c */
char	*sh_statusline(t_sh *);

#endif
