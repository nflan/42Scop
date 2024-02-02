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

GLSLC =			/mnt/nfs/homes/nflan/sgoinfre/bin/glslc

INC =			$(addsuffix .hpp, $(addprefix $(INC_DIR), Buffer Camera Descriptors Device Display FrameInfo GameObject KeyboardMovementController Loader Material Mesh Model Pipeline PointLightSystem Renderer RenderSystem SwapChain tools UniformBufferObject Window))

SRC =			$(SRC_FT:%=$(SRC_DIR)/%.cpp)
SRCB =			$(SRCB_FT:%=$(SRCB_DIR)/%.cpp)

OBJ =			$(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)
OBJB =			$(SRCB:$(SRCB_DIR)%.cpp=$(OBJB_DIR)%.o)

CXX =	g++ $(CXXFLAGS)

RM =	rm -fr

CXXFLAGS =	-std=c++20 -O3 -g3# -DNDEBUG

GLM_INCLUDE_PATH = /home/nflan/bin/glm

LDFLAGS =	-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -I$(GLM_INCLUDE_PATH)

# VulkanTest:	main.cpp
# g++ $(CXXFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.cpp.o:
	$(CXX) -c $< -o $(<:.cpp=.o)

#HOW TO LIST .c 
#	ls -l | awk '{print $9}' | grep -E ".c$"| sed "s/\.c/ \\\/g" | sed '$s/\\$//g'

SRC_FT =	Buffer \
			Camera \
			Descriptors \
			Device \
			Display \
			GameObject \
			KeyboardMovementController \
			Loader \
			Material \
			Mesh \
			Model \
			Pipeline \
			PointLightSystem \
			tools \
			Renderer \
			RenderSystem \
			scop \
			SwapChain \
			Vertex \
			Window 

SRCB_FT =

vertSources = $(shell find shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))

all: $(NAME)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ) : $(INC) | $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< -o $@

$(NAME): $(vertObjFiles) $(fragObjFiles)

$(NAME): $(INC) $(OBJ_DIR) $(SRC) $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

%.vert.spv: %.vert
	${GLSLC} $< -o $@

%.frag.spv: %.frag
	${GLSLC} $< -o $@

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
