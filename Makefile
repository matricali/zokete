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

all: dirs $(NAME)

dirs:
	mkdir -p obj

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: install
install: $(NAME)
	mkdir -p $(DESTDIR)$(PREFIX)$(BINDIR)
	cp $(NAME) $(DESTDIR)$(PREFIX)$(BINDIR)/$(NAME)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(BINDIR)/$(NAME)
