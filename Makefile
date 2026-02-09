NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Client.cpp Server.cpp
OBJS = $(SRCS:%.cpp=obj/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

obj:
	mkdir -p obj

obj/%.o: %.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)
	rm -rf obj

re: fclean all

.PHONY: all clean fclean re