#ifndef LOADER_HPP
#define LOADER_HPP

#include <vector>
#include <string>
#include "Material.hpp"
#include "Mesh.hpp"
#include "Vertex.hpp"

class Loader
{
	public:
		// Default Constructor
		Loader()
		{

		}
		~Loader()
		{
			LoadedMeshes.clear();
		}

		// Load a file into the loader
		//
		// If file is loaded return true
		//
		// If the file is unable to be found
		// or unable to be loaded return false
        bool    LoadFile(const std::string& Path);

        std::string             _file;
        std::string             _path;
        std::string             _mtlFile;
        // Loaded Mesh Objects
        std::vector<Meshou>         LoadedMeshes;
        // Loaded Vertex Objects
        std::vector<Vertex>         LoadedVertices;
        // Loaded Index Positions
        std::vector<uint32_t>       LoadedIndices;
        // Loaded Material Objects
        std::vector<Material>       LoadedMaterials;
    private:
    	// Generate vertices from a list of positions, 
		//	tcoords, normals and a face line
		void    GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
			const std::vector<glm::vec3>& iPositions,
			const std::vector<glm::vec2>& iTCoords,
			const std::vector<glm::vec3>& iNormals,
			std::string icurline);
        // Triangulate a list of vertices into a face by printing
		//	inducies corresponding with triangles within it
		void    VertexTriangluation(std::vector<unsigned int>& oIndices,
			const std::vector<Vertex>& iVerts);
        // Load Materials from .mtl file
		bool    LoadMaterials(std::string path);
};

namespace tools {
    // glm::vec3 Cross Product
    glm::vec3 ft_CrossV3(const glm::vec3 a, const glm::vec3 b);

    // glm::vec3 Magnitude Calculation
    float ft_MagnitudeV3(const glm::vec3 in);

    // glm::vec3 DotProduct
    float ft_DotV3(const glm::vec3 a, const glm::vec3 b);

    // Angle between 2 glm::vec3 Objects
    float ft_AngleBetweenV3(const glm::vec3 a, const glm::vec3 b);

    // Projection Calculation of a onto b
    glm::vec3 ft_ProjV3(const glm::vec3 a, const glm::vec3 b);

    // glm::vec3 Multiplication Opertor Overload
    glm::vec3 operator*(const float& left, const glm::vec3& right);

    // A test to see if P1 is on the same side as P2 of a line segment ab
    bool ft_SameSide(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b);

    // Generate a cross produect normal for a triangle
    glm::vec3 ft_GenTriNormal(glm::vec3 t1, glm::vec3 t2, glm::vec3 t3);

    // Check to see if a glm::vec3 Point is within a 3 glm::vec3 Triangle
    bool ft_inTriangle(glm::vec3 point, glm::vec3 tri1, glm::vec3 tri2, glm::vec3 tri3);

    // Split a String into a string array at a given token
    void ft_split(const std::string &in,
        std::vector<std::string> &out,
        std::string token);

    // Get tail of string after first token and possibly following spaces
    std::string ft_tail(const std::string &in);

    // Get first token of string
    std::string ft_firstToken(const std::string &in);

    // Get element at given index position
    template <class T>
    const T & ft_getElement(const std::vector<T> &elements, std::string &index);
}

#endif