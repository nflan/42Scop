/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 18:06:10 by nflan             #+#    #+#             */
/*   Updated: 2023/10/24 18:10:16 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOOLS_HPP
#define TOOLS_HPP

#define GLFW_INCLUDE_VULKAN
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "Vertex.hpp"

#define ROTATION 0.0005f

extern bool         QUIT;
extern bool         ROBJ;
extern short	    WAY;
extern float        ROTX;
extern float        ROTY;
extern float        ROTZ;
extern glm::mat4    ROTATE;

namespace std {
// from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };
}

struct  Material
{
    Material()
    {
        _ka = glm::vec3(0.f);
        _kd = glm::vec3(0.f);
        _ks = glm::vec3(0.f);
        _ke = glm::vec3(0.f);
        _illum = 1;
        _ns = 0.f;
        _ni = 0.f;
        _d = 0.f;
    }
    std::string         _name;//name
    glm::vec3           _ka;//ambient color
    glm::vec3           _kd;//diffuse color
    glm::vec3           _ks;//spectacular color
    glm::vec3           _ke;//couleur émissive (emissive)
    int                 _illum;//illum = 1(0) a flat material with no specular highlights, illum = 2(1) denotes the presence of specular highlights
    float               _ns;//shininess of the material
    float               _ni;//densite optique
    float               _d;//transparency d or Tr
    std::string         _mapKa;//names a file containing an Ambient Texture Map, which should just be an ASCII dump of RGB values
    std::string         _mapKd;//names a file containing a Diffuse Texture Map, which should just be an ASCII dump of RGB values
    std::string         _mapKs;//names a file containing a Specular Texture Map, which should just be an ASCII dump of RGB values
    std::string         _mapNs;//names a file containing a Specular Hightlight Map, which should just be an ASCII dump of RGB values
    std::string         _mapD;//names a file containing an Alpha Texture Map, which should just be an ASCII dump of RGB values
    std::string         _mapBump;//names a file containing a Bump Map, which should just be an ASCII dump of RGB values
};


struct Mesh
{
    // Default Constructor
    Mesh() {}
    // Variable Set Constructor
    Mesh(std::vector<Vertex>& _Vertices, std::vector<unsigned int>& _Indices)
    {
        _vertices = _Vertices;
        _indices = _Indices;
    }
    // Mesh Name
    std::string                 _meshName;
    // Vertex List
    std::vector<Vertex>         _vertices;
    // Index List
    std::vector<unsigned int>   _indices;
    // Material
    Material                    _meshMaterial;
};

struct  Texture {
    Texture() = delete;
    Texture(VkDevice& device): _device(device) {}
    ~Texture()
    {
        if (_sampler != nullptr)
            vkDestroySampler(_device, _sampler, nullptr);
        if (_imageView != nullptr)
		    vkDestroyImageView(_device, _imageView, nullptr);
        if (_image != nullptr)
		    vkDestroyImage(_device, _image, nullptr);
        if (_imageMemory != nullptr)
		    vkFreeMemory(_device, _imageMemory, nullptr);
    }
	VkImage			                    _image = nullptr;
	VkImageView		                    _imageView = nullptr;
	VkDeviceMemory	                    _imageMemory = nullptr;
	VkSampler		                    _sampler = nullptr;
	uint32_t		                    _mipLevels;
    VkDevice                            _device;
};

void            printMaterial(Material);
bool            testObjFile(std::string);
bool            testOpenFile(std::string);
bool            isTexFile(std::string);
std::ostream &  operator<<(std::ostream& o, glm::vec3 vec);

#endif
