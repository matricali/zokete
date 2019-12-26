CC	= gcc

CFLAGS	+= -Wall -g -std=gnu99 -O2 -Iinclude
LDFLAGS	+=

NAME	= zoketed
SRCS	:= zokete.c logger.c server.c
OBJS	:= $(SRCS:%.c=obj/%.o)

$(NAME): $(OBJS)
	@$(CC) $(OBJS) $(LDFLAGS) -o $@
	@echo "Linking complete!"

$(OBJS): obj/%.o : src/%.c
	@$(CC) $(USER_DEFINES) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: all
all: dirs $(NAME)

.PHONY: dirs
dirs:
	mkdir -p obj

.PHONY: clean
clean:
	rm -f $(OBJS)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all

.PHONY: install
install: $(NAME)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(NAME) $(DESTDIR)$(PREFIX)/bin/$(NAME)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(BINDIR)/$(NAME)
