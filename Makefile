# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: nflan <marvin@42.fr>                       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/10/23 18:11:08 by nflan             #+#    #+#              #
#                                                                              #
# **************************************************************************** #

NAME =	scop
NAMEB =	scop_bonus

INC_DIR =		incs/
INCB_DIR =		incs_bonus/
OBJ_DIR =		obj
OBJB_DIR =		obj_bonus
SRC_DIR =		srcs
SRCB_DIR =		srcs_bonus

INC =			$(addsuffix .hpp, $(addprefix $(INC_DIR), scop Display Vertex QueueFamilyIndices SwapChainSupportDetails Mesh tools))

SRC =			$(SRC_FT:%=$(SRC_DIR)/%.cpp)
SRCB =			$(SRCB_FT:%=$(SRCB_DIR)/%.cpp)

OBJ =			$(SRC:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)
OBJB =			$(SRCB:$(SRCB_DIR)%.c=$(OBJB_DIR)%.o)

CXX =	g++ $(CXXFLAGS)

RM =	rm -fr

CXXFLAGS =	-std=c++20 -O3 -g3# -DNDEBUG

GLM_INCLUDE_PATH = /mnt/nfs/homes/nflan/sgoinfre/bin/glm
STB_INCLUDE_PATH = /mnt/nfs/homes/nflan/sgoinfre/bin/stb
OBJLOADER_INCLUDE_PATH = /mnt/nfs/homes/nflan/sgoinfre/bin/tinyobjloader

LDFLAGS =	-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -I$(GLM_INCLUDE_PATH) -I$(STB_INCLUDE_PATH) -I$(OBJLOADER_INCLUDE_PATH)

# VulkanTest:	main.cpp
# g++ $(CXXFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.c.o:
	$(CXX) -c $< -o $(<:.cpp=.o)

#HOW TO LIST .c 
#	ls -l | awk '{print $9}' | grep -E ".c$"| sed "s/\.c/ \\\/g" | sed '$s/\\$//g'

SRC_FT =	scop Display Vertex Mesh

SRCB_FT =

all: $(NAME)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ) : $(INC) | $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) -c $< -o $@

$(NAME): $(INC) $(OBJ_DIR) $(SRC) $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

bonus: $(NAMEB)

$(OBJB_DIR):
	mkdir -p $@

$(OBJB) : $(INCB) | $(OBJB_DIR)

$(OBJB_DIR)/%.o: $(SRCB_DIR)/%.c
	$(CXX) -c $< -o $@

$(NAMEB): $(OBJB_DIR) $(SRCB) $(OBJB)
	$(CXX) $(OBJB) -o $(LDFLAGS) $@

clean:
	@$(RM) $(OBJ_DIR)
	@$(RM) $(OBJB_DIR)
	@echo "Cleaned object"

fclean: clean
	@$(RM) $(NAME)
	@$(RM) $(NAMEB)
	@echo "Cleaned program"

re: fclean all

define print_aligned_coffee
    @t=$(NAME); \
	l=$${#t};\
	i=$$((8 - l / 2));\
	echo "             \0033[1;32m\033[3C\033[$${i}CAnd Your program \"$(NAME)\" "
endef

coffee: all clean
	@echo ""
	@echo "                                {"
	@echo "                             {   }"
	@echo "                              }\0033[1;34m_\0033[1;37m{ \0033[1;34m__\0033[1;37m{"
	@echo "                           \0033[1;34m.-\0033[1;37m{   }   }\0033[1;34m-."
	@echo "                          \0033[1;34m(   \0033[1;37m}     {   \0033[1;34m)"
	@echo "                          \0033[1;34m| -.._____..- |"
	@echo "                          |             ;--."
	@echo "                          |            (__  \ "
	@echo "                          |             | )  )"
	@echo "                          |   \0033[1;96mCOFFEE \0033[1;34m   |/  / "
	@echo "                          |             /  / "
	@echo "                          |            (  / "
	@echo "                          \             | "
	@echo "                            -.._____..- "
	@echo ""
	@echo ""
	@echo "\0033[1;32m\033[3C                    Take Your Coffee"
	$(call print_aligned_coffee)

.SECONDARY: $(OBJ) $(OBJ_DIR) $(OBJB) $(OBJB_DIR)
.PHONY: all clean fclean re coffee bonus
