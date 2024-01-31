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
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>

#define ROTATION 0.0005f

extern bool QUIT;
extern bool ROBJ;
extern short	WAY;
extern float    ROTX;
extern float    ROTY;
extern float    ROTZ;

namespace std {

// from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };

}

struct  Material {
    std::string         _newmtl;//name
    std::vector<float>  _ka;//ambient color
    std::vector<float>  _kd;//diffuse color
    std::vector<float>  _ks;//spectacular color
    std::vector<float>  _ke;//couleur émissive (emissive)
    int                 _illum;//illum = 1(0) a flat material with no specular highlights, illum = 2(1) denotes the presence of specular highlights
    float               _ns;//shininess of the material
    float               _ni;//densite optique
    float               _d;//transparency d or Tr
    std::string         _mapKa;//names a file containing a texture map, which should just be an ASCII dump of RGB values
};

struct  Texture {
	VkImage			                    _image;
	VkImageView		                    _imageView;
	VkDeviceMemory	                    _imageMemory;
	VkSampler		                    _sampler;
	uint32_t		                    _mipLevels;
};

void    printMaterial(Material);

#endif
