# **************************************************************************** #
#                                   CONFIG                                     #
# **************************************************************************** #

NAME        = webserv
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
RM          = rm -f

SRC_DIR     = src
OBJ_DIR     = obj
CGI_DIR		= www/tetris/cgi-bin

SRC         = $(SRC_DIR)/main.cpp \
				$(SRC_DIR)/Location.cpp \
				$(SRC_DIR)/Client.cpp \
				$(SRC_DIR)/Server.cpp \
				$(SRC_DIR)/Request.cpp \
				$(SRC_DIR)/Config.cpp \
				$(SRC_DIR)/Response.cpp \
				$(SRC_DIR)/ServerConfig.cpp

OBJ         = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# **************************************************************************** #
#                                   RULES                                      #
# **************************************************************************** #

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "✅ Build complete: $(NAME)"
	make -C $(CGI_DIR)

# Rule to compile .cpp → .o inside objs/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled: $< → $@"

clean:
	$(RM) -r $(OBJ_DIR)
	@echo "🧹 Object files removed"
	make clean -C $(CGI_DIR)

fclean: clean
	$(RM) $(NAME)
	@echo "🗑️  Executable removed"
	make fclean -C $(CGI_DIR)

re: fclean all

# **************************************************************************** #
#                                   UTILS                                      #
# **************************************************************************** #

.PHONY: all clean fclean re