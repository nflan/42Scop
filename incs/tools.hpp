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

#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtx/hash.hpp>

#define ROTATION 0.0005f

extern bool QUIT;
extern bool ROBJ;
extern short	WAY;
extern float    ROTX;
extern float    ROTY;
extern float    ROTZ;

struct Texture {
	//textures d'image (pixels -> texels)
	uint32_t					_mipLevels;
	VkImage						_textureImage;
	VkDeviceMemory				_textureImageMemory;
	//reference a la vue pour la texture
	VkImageView					_textureImageView;
	VkSampler					_textureSampler;
};

namespace std {

// from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };

}

#endif
