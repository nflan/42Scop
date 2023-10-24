# ft_scop
Basic GPU rendering with OpenGL

Graphic rendering with: Vulkan
Base code: C++

II.2 What you need to do
Your goal is to create a small program that will show a 3D object conceived with a modelization program like Blender. The 3D object is stored in a .obj file. You will be at least
in charge of parsing to obtain the requested rendering.
In a window, your 3D object will be displayed in perspective (which means that what
is far must be smaller), rotate on itself around its main symmetrical axis (middle of the
object basically...). By using various colors, it must be possible to distinguish the various
sides. The object can be moved on three axis, in both directions.
Finally, a texture must be applicable simply on the object when we press a dedicated
key, and the same key allows us to go back to the different colors. A soft transition
between the two is requested.

Resources are available on the projet on 42 intra


POUR COMPILER
Verifier les installations:

- https://vulkan-tutorial.com/fr/Environnement_de_developpement#page_Paquets-Vulkan

- glslc:
	https://github.com/google/shaderc/blob/main/downloads.md
	prendre GCC
	A mettre dans bin et si pas droit bah rajouter le path

- Ensuite, s'il manque glm:
	https://glm.g-truc.net/0.9.6/index.html
	puis rajouter dans le makefile -I/PATH/glm
