/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SwapChainSupportDetails.hpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/25 15:25:34 by nflan             #+#    #+#             */
/*   Updated: 2023/10/25 15:27:39 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SWAPCHAINSUPPORTDETAILS_HPP
#define SWAPCHAINSUPPORTDETAILS_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

struct SwapChainSupportDetails { // ces trois variables contiennent les infos:
	VkSurfaceCapabilitiesKHR capabilities; // Possibilités basiques de la surface (nombre min/max d'images dans la swap chain, hauteur/largeur min/max des images)
	std::vector<VkSurfaceFormatKHR> formats; // Format de la surface (format des pixels, palette de couleur)
	std::vector<VkPresentModeKHR> presentModes; // Mode de présentation disponibles
};

#endif
