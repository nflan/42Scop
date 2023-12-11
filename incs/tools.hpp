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

#endif
