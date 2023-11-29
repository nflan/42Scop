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
            glm::vec2   tex;
            double    U, V;
            vt >> U; vt >> V;
            tex = glm::vec2(U,V);
            this->_textures.push_back(tex);
        }
        //check for light
        else if (line.substr(0,2)=="vn")
        {
            std::istringstream  vn(line.substr(3));
            glm::vec3   norm;
            double x,y,z;
            vn >> x; vn >> y; vn >> z;
            // std::cerr << "x = " << x << " y = " << y << " z = " << z <<std::endl;
            norm = glm::vec3(x,y,z);
            this->_normals.push_back(norm);
        }
        //check for faces
        else if (line.substr(0,2)=="f ")
        {
            //vector de points
            std::istringstream  f(line.substr(2));
            // std::vector<uint32_t>    faces;
            std::vector<std::string>    elements;
            // std::string     tmpline;
            uint32_t        tmp;

            for (std::string element; f >> element;)
            {
                elements.push_back(element);
            }
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (elements[i].find('/') != std::string::npos)
                {
                    // std::cerr << elements[i] << std::endl;
                    std::string   element = elements[i];
                    size_t  info = 0;         
                    std::string    vert, uv, norm;
                    int d = 0;
                    // std::istringstream  spliting(elements[i]);
                    for (std::string::iterator it = element.begin(); it != element.end(); it++)
                    {
                        // std::cerr << "info " << info << " d = " << d << std::endl;
                        if (*it != '/')
                        {
                            // std::cerr << "*it = " << *it << std::endl;
                            if (info == 0)
                            {
                                vert.push_back(*it);
                            }
                            else if (info == 1 && this->_textures.size())
                            {
                                // std::cerr << "push dans textures" << std::endl;
                                uv.push_back(*it);
                            }
                            else if (info == 2 && this->_normals.size())
                            {
                                // std::cerr << "push dans normals" << std::endl;
                                norm.push_back(*it);
                            }
                        }
                        else if (*it == '/')
                        {
                            while (it != element.end() && *it + 1 == '/')
                            {
                                it++;
                                info++;
                            }
                            info++;
                        }
                        d++;
                    }
                    // std::cerr << "vert = " << vert << std::endl;
                    this->_faceIndex.push_back(static_cast<uint32_t>(std::stoul(vert)) - 1);
                    if (uv.size())
                    {
                        // std::cerr << "uv = " << uv << std::endl;
                        // std::cerr << "push back dans texture -> " << static_cast<uint32_t>(std::stoul(uv)) << std::endl;
                        this->_textureIndex.push_back(static_cast<uint32_t>(std::stoul(uv)));
                        // std::cerr << this->_textureIndex.size() << std::endl;
                    }
                    if (norm.size())
                    {
                        // std::cout << "norm = " << norm << std::endl;
                        // std::cerr << "push back dans norm -> " << static_cast<uint32_t>(std::stoul(norm)) << std::endl;
                        this->_normalsIndex.push_back(static_cast<uint32_t>(std::stoul(norm)));
                        // std::cerr << this->_normalsIndex.size() << std::endl;
                    }
                }
                else
                {
                    while (elements.size() >= 3)
                    {
                        this->_faceIndex.push_back(static_cast<uint32_t>(std::stoul(elements[0])) - 1);
                        this->_faceIndex.push_back(static_cast<uint32_t>(std::stoul(elements[1])) - 1);
                        this->_faceIndex.push_back(static_cast<uint32_t>(std::stoul(elements[2])) - 1);
                        elements.erase(elements.begin() + 1);
                    }
                }
            }
            // while (f.tellg() != -1)
            // {
            //     tmpline.clear();
            //     tmp = 0;
            //     f >> tmpline;
            //     elements = 
            //     int i = 0;
            //     while (tmpline.find("/") != std::string::npos)
            //     {
            //         i++;
            //         tmp = static_cast<uint32_t>(std::stoul(tmpline.substr(0, tmpline.find("/"))));
            //         tmpline = tmpline.erase(0, tmpline.find("/") + 1);
            //         std::cout << tmp << std::endl;
            //         std::cout << tmpline << std::endl;
            //         std::cerr << "i = " << i << " inf?" << std::endl;
            //         // tmp--;
            //     // faces.push_back(tmp);
            //     }
            //     tmp = static_cast<uint32_t>(std::stoul(tmpline.substr(0, tmpline.find("/"))));
            //     // tmp--;
            //     std::cout << tmp << std::endl;
            //     // faces.push_back(tmp);
            // }
            // while (faces.size() >= 3)
            // {
            //     this->_faceIndex.push_back(faces[0]);
            //     this->_faceIndex.push_back(faces[1]);
            //     this->_faceIndex.push_back(faces[2]);
            //     faces.erase(faces.begin() + 1);
            // }
            // std::cout << "faceindex " << this->_faceIndex.size() << std::endl;
            // this->_textureIndex.push_back(A);
            // this->_textureIndex.push_back(B);
            // this->_textureIndex.push_back(C);
        }

    }
    // std::cout << "uvIndex.size() " << _textureIndex.size() << " _uv.size() " << _texture.size() << std::endl;
    // std::cout << "normalIndex.size() " << _normalsIndex.size() << " _normals.size() " << _normals.size() << std::endl;
    //calculate all mesh vertices using face index
    // std::cout << "textureIndex[0] = " << _textureIndex[0] <<std::endl;
    for(unsigned int i = 0; i < this->_faceIndex.size(); i++)
    {
        // std::cout << "i = " << i << std::endl;
        glm::vec3   meshData;
        glm::vec2   texData;
        glm::vec3   normData;

        meshData = glm::vec3(this->_vertices[this->_faceIndex[i]].x, this->_vertices[this->_faceIndex[i]].y, this->_vertices[this->_faceIndex[i]].z);
        if (_textures.size() && _textures.size() >= i)
        {
            // std::cout << "this->_texture[_textureIndex[i]].x " << this->_texture[_textureIndex[i]].x << std::endl;
            // std::cout << "this->_texture[_textureIndex[i]].x " << this->_texture[_textureIndex[i]].x << std::endl;
            texData = glm::vec2(this->_textures[this->_textureIndex[i]].x, this->_textures[this->_textureIndex[i]].y);
            this->_texCoord.push_back(texData);
        }
        if (_normals.size() && _normals.size() >= i)
        {
            // std::cout << "this->_normals[_normalsIndex[i]].x " << this->_normals[_normalsIndex[i]].x << std::endl;
            // std::cout << "this->_normals[_normalsIndex[i]].y " << this->_normals[_normalsIndex[i]].y << std::endl;
            // std::cout << "this->_normals[_normalsIndex[i]].z " << this->_normals[_normalsIndex[i]].z << std::endl;
            normData = glm::vec3(this->_normals[this->_normalsIndex[i]].x, this->_normals[this->_normalsIndex[i]].y, this->_normals[this->_normalsIndex[i]].z);
            this->_normCoord.push_back(normData);
        }
        this->_meshVertices.push_back(meshData);
    }

}

//function for returing mesh vertex data
std::vector<glm::vec3>  Mesh::getMeshVertices() { return this->_meshVertices; }
std::vector<glm::vec3>  Mesh::getVertices() { return this->_vertices; }
std::vector<uint32_t>   Mesh::getFaceIndex() { return this->_faceIndex; }
std::vector<glm::vec2>  Mesh::getTextures() { return this->_textures; }
std::vector<uint32_t>   Mesh::getTextureIndex() { return this->_textureIndex; }
std::vector<glm::vec2>  Mesh::getTexCoord() { return this->_texCoord; }
std::vector<glm::vec3>  Mesh::getNormals() { return this->_normals; }
std::vector<glm::vec3>  Mesh::getNormCoord() { return this->_normCoord; }