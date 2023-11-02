/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mesh.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/30 14:37:31 by nflan             #+#    #+#             */
/*   Updated: 2023/10/30 14:37:32 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Mesh.hpp"

Mesh::Mesh() {}

void Mesh::LoadObjModel(const std::string &filename)
{
    std::ifstream in(filename, std::ios::in);
    if (!in)
    {
        std::cerr << "Cannot open " << filename << std::endl;
        exit(1);
    }
    std::string line;
    while (std::getline(in, line))
    {
        if (line.substr(0,2)=="v ")
        {
            std::istringstream  v(line.substr(2));
            glm::vec3   vert;
            double x,y,z;
            v >> x; v >> y; v >> z;
            vert = glm::vec3(x,y,z);
            this->_vertices.push_back(vert);
        }
        //check for texture co-ordinate
        else if (line.substr(0,2)=="vt")
        {
            std::istringstream  vt(line.substr(3));
            glm::vec2 tex;
            int    U, V;
            vt >> U; vt >> V;
            tex=glm::vec2(U,V);
            this->_texture.push_back(tex);
        }
        //check for light
        else if (line.substr(0,2)=="vn")
        {
            std::istringstream  vn(line.substr(3));
            glm::vec3 norm;
            double x,y,z;
            vn >> x; vn >> y; vn >> z;
            norm = glm::vec3(x,y,z);
            this->_normals.push_back(norm);
        }
        //check for faces
        else if (line.substr(0,2)=="f ")
        {//vector de points
            std::istringstream  f(line.substr(2));
            std::vector<uint32_t>    faces;
            uint32_t    tmp;

            while (f.tellg() != -1)
            {
                tmp = 0;

                f >> tmp;
                tmp--;
                faces.push_back(tmp);
            }
            while (faces.size() >= 3)
            {
                std::cerr << "j'ajoute des f" << std::endl;
                this->_faceIndex.push_back(faces[0]);
                this->_faceIndex.push_back(faces[1]);
                this->_faceIndex.push_back(faces[2]);
                faces.erase(faces.begin() + 1);
            }
            std::cout << "faceindex " << this->_faceIndex.size() << std::endl;
            // this->_textureIndex.push_back(A);
            // this->_textureIndex.push_back(B);
            // this->_textureIndex.push_back(C);
        }

    }
    //calculate all mesh vertices using face index
    for(unsigned int i = 0; i < this->_faceIndex.size(); i++)
    {
        glm::vec3   meshData;
        glm::vec2   texData;

        meshData = glm::vec3(this->_vertices[this->_faceIndex[i]].x, this->_vertices[this->_faceIndex[i]].y, this->_vertices[this->_faceIndex[i]].z);
        if (_texture.size() && _textureIndex[i])
        {
            texData = glm::vec2(this->_texture[this->_textureIndex[i]].x, this->_texture[this->_textureIndex[i]].y);
            this->_texCoord.push_back(texData);

        }
        this->_meshVertices.push_back(meshData);
    }

}

//function for returing mesh vertex data
std::vector<glm::vec3>  Mesh::getMeshVertices() { return this->_meshVertices; }
std::vector<glm::vec3>  Mesh::getVertices() { return this->_vertices; }
std::vector<uint32_t>   Mesh::getFaceIndex() { return this->_faceIndex; }
std::vector<glm::vec2>  Mesh::getTexture() { return this->_texture; }
std::vector<uint32_t>   Mesh::getTextureIndex() { return this->_textureIndex; }
std::vector<glm::vec2>  Mesh::getTexCoord() { return this->_texCoord; }
std::vector<glm::vec3>  Mesh::getNormals() { return this->_normals; }