/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Triangle.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:11:00 by nflan             #+#    #+#             */
/*   Updated: 2023/10/24 19:09:03 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "tools.hpp"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class Triangle
{
	public:
		Triangle( void );
		void	run( void );
		Triangle( const Triangle & );
		~Triangle( void );

	private:
		void	initWindow( void ); // initWindow
		void	initVulkan( void ); // initImg
		void	mainLoop( void ); // boucle
		void	cleanup( void ); // clean
		void	createInstance( void ); // create instance vulkan
		bool	checkValidationLayerSupport( void ); //check layer (protections)
		std::vector<const char*>	getRequiredExtensions();
		void	setupDebugMessenger( void );

		GLFWwindow*	_window;
		VkInstance	_instance;
		VkDebugUtilsMessengerEXT	_debugMessenger;

};

#endif
