CC		= cc
CFLAGS	= -Wall -Wextra -Werror
LFLAGS	= -lreadline
SRCS	= \
	./shell-utils.c \
	./shell-parse.c \
	./shell-exec.c \
	./shell-builtin.c \
	./shell-redirect.c \
	./shell-var.c \
	./shell-rc.c \
	./shell-kvll.c \
	./shell-alias.c \
	./shell-statusline.c \
	./shell-getopt.c \
	./shell.c
OBJS	= $(SRCS:.c=.o)
NAME	= shell

.PHONY : all

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY : clean

clean:
	rm -f $(NAME)
	rm -f $(OBJS)

.PHONY : debug

debug : CFLAGS += -ggdb3 -O0
debug : all

.PHONY : release

release : CFLAGS += -O3
release : all

.PHONY : install

install : $(NAME)
	cp -f $(NAME) /usr/local/bin

.PHONY : uninstall

uninstall :
	rm -f /usr/local/bin/$(NAME)
