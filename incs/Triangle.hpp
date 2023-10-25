/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Triangle.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:11:00 by nflan             #+#    #+#             */
/*   Updated: 2023/10/25 16:13:26 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "tools.hpp"
#include "QueueFamilyIndices.hpp"
#include "SwapChainSupportDetails.hpp"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <map>
#include <set>
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
		std::vector<const char*>	getRequiredExtensions(); // recup extensions debug
		void	setupDebugMessenger( void ); // debug
		void	populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void	pickPhysicalDevice(); // recuperations de toutes les cg
		bool	isDeviceSuitable(VkPhysicalDevice device); // voir si la cg correspond a ce qu'on va faire
		int		rateDeviceSuitability(VkPhysicalDevice device); // score en fonction des fonctionnalites de la cg
		QueueFamilyIndices	findQueueFamilies(VkPhysicalDevice device); // trouver la famille de queue "graphique"
		void	createLogicalDevice();
		void	createSurface( void ); // creation de la surface de fenetre multiplateforme
		bool	checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails	querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR	chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR	chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D	chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); // choisir le format de l'image, souvent taille de la fenetre, le width et height sont limites par cette derniere, mais s'ils sont en uint32 max, alors on peut choisir ce qu'on desire
		void	createSwapChain(); //recup les informations des precedentes fonctions sur la swap

		GLFWwindow*	_window;
		VkInstance	_instance;
		VkDebugUtilsMessengerEXT	_debugMessenger;
		VkPhysicalDevice	_physicalDevice; // auto detruit a la fin de l'instance
		std::vector<VkPhysicalDevice>	_devices;
		VkDevice	_vkDevice; // specifier ce dont nous allons avoir besoin pour logical device
		VkQueue		_graphicsQueue; // auto detruit au destroy de vkdevice
		VkSurfaceKHR	_surface;
		VkQueue		_presentQueue;
		std::vector<const char*>	_deviceExtensions;
		VkSwapchainKHR	_swapChain;
		std::vector<VkImage>	_swapChainImages; // images de la swap chain (auto del avec la swapchain)
		VkFormat	_swapChainImageFormat;
		VkExtent2D	_swapChainExtent;
};

#endif
