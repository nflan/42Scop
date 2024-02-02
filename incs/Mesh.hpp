/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mesh.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/30 14:19:21 by nflan             #+#    #+#             */
/*   Updated: 2023/10/30 14:19:22 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESH_HPP
#define MESH_HPP

#define GLFW_INCLUDE_VULKAN
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include "Material.hpp"
#include "Vertex.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

struct Meshou
{
    // Default Constructor
    Meshou()
    {

    }
    // Variable Set Constructor
    Meshou(std::vector<Vertex>& _Vertices, std::vector<unsigned int>& _Indices)
    {
        Vertices = _Vertices;
        Indices = _Indices;
    }
    // Mesh Name
    std::string                 MeshName;
    // Vertex List
    std::vector<Vertex>         Vertices;
    // Index List
    std::vector<unsigned int>   Indices;
    // Material
    Material                    MeshMaterial;
};

class Mesh
{
    public:
        Mesh();//constructor
        
        void                    loadObjModel(const std::string &filename);//function to load obj model
        std::vector<glm::vec3>  getMeshVertices();//return the vertices of mesh data
        std::vector<glm::vec3>  getVertices();
        std::vector<uint32_t>   getFaceIndex();
        std::vector<glm::vec2>  getTextures();
        std::vector<uint32_t>   getTextureIndex();
        std::vector<glm::vec2>  getTexCoord();
        std::vector<glm::vec3>  getNormals();
        std::vector<glm::vec3>  getNormCoord();
        std::string             getMtlFile();

    private:
        std::vector<glm::vec3>  _vertices;//to store vertex information of 3D model started with v
        std::vector<glm::vec3>  _meshVertices;//to store all 3D model face vertices
        std::vector<uint32_t>   _faceIndex;//to store the number of face index started with f
        std::vector<glm::vec3>  _normals;
        std::vector<uint32_t>   _normalsIndex;
        std::vector<glm::vec3>  _normCoord;
        std::vector<glm::vec2>  _textures;
        std::vector<uint32_t>   _textureIndex;
        std::vector<glm::vec2>  _texCoord;
        std::string             _mtlFile;
};

#endif
