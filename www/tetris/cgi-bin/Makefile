NAME := save-score.cgi
SRCS := save-score.cpp
OBJS = $(SRCS:.cpp=.o)
CC = c++ -Wall -Wextra -Werror -std=c++98 #-fsanitize=address,undefined -fno-omit-frame-pointer -g -O0
RM = rm -f
all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $(NAME) $(LDFLAGS)
%.o: %.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
clean:
	@$(RM) $(OBJS)
fclean: clean
	@$(RM) $(NAME)
re: fclean all
.PHONY: all clean fclean re