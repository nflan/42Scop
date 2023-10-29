/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:11:00 by nflan             #+#    #+#             */
/*   Updated: 2023/10/27 12:17:56 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>
#include "tools.hpp"
#include "QueueFamilyIndices.hpp"
#include "SwapChainSupportDetails.hpp"
#include "UniformBufferObject.hpp"
#include "Vertex.hpp"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <fstream>
#include <map>
#include <set>
#include <cstdlib>
#include <cstring>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class Display
{
	public:
		Display( void );
		Display( const Display & );
		Display &	operator=( const Display& o );
		~Display( void );

		void	run( void );
		bool	framebufferResized = false;
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
		void	createImageViews();
		void	createDescriptorSetLayout();
		void	createGraphicsPipeline(); //https://vulkan-tutorial.com/fr/Dessiner_un_Display/Pipeline_graphique_basique/Introduction
		VkShaderModule	createShaderModule(const std::vector<char>& code); //avant d'envoyer les infos des shaders dans la pipeline
		void	createRenderPass();//https://vulkan-tutorial.com/fr/Dessiner_un_Display/Pipeline_graphique_basique/Render_pass
		void	createFramebuffers();
		void	createCommandPool();
		void	createCommandBuffers();
		void	drawFrame();
		void	createSyncObjects();
		void	recreateSwapChain();
		void	cleanupSwapChain();
		void	createVertexBuffer();
		void	createIndexBuffer();
		void	createUniformBuffers();
		void 	updateUniformBuffer(uint32_t currentImage);
		void	createDescriptorPool();
		void	createDescriptorSets();
		uint32_t	findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void 	createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void 	copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		//images mais a voir pour changer plus tard
		void	createTextureImage();
		void	createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		VkCommandBuffer	beginSingleTimeCommands();
		void	endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void	transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void	copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void	createTextureImageView();
		VkImageView	createImageView(VkImage image, VkFormat format);
		void	createTextureSampler();


		GLFWwindow*					_window;

		VkInstance					_instance;
		VkDebugUtilsMessengerEXT	_debugMessenger;
		VkSurfaceKHR				_surface;

		VkPhysicalDevice			_physicalDevice = VK_NULL_HANDLE; // auto detruit a la fin de l'instance
		VkDevice					_device; // specifier ce dont nous allons avoir besoin pour logical device

		VkQueue						_graphicsQueue; // auto detruit au destroy de vkdevice
		VkQueue						_presentQueue;

		VkSwapchainKHR				_swapChain;
		std::vector<VkImage>		_swapChainImages; // images de la swap chain (auto del avec la swapchain)
		VkFormat					_swapChainImageFormat;
		VkExtent2D					_swapChainExtent;
		std::vector<VkImageView>	_swapChainImageViews;
		//FRAMEBUFFERS
		std::vector<VkFramebuffer>	_swapChainFramebuffers;

		//RENDER PASS
		VkRenderPass				_renderPass;
		VkDescriptorSetLayout 		_descriptorSetLayout;
		VkPipelineLayout			_pipelineLayout;
		VkPipeline					_graphicsPipeline;

		//COMMAND POOLS
		VkCommandPool				_commandPool;

		//VertexBuffers
		VkBuffer					_vertexBuffer;
		VkDeviceMemory				_vertexBufferMemory;
		VkBuffer 					_indexBuffer;
		VkDeviceMemory 				_indexBufferMemory;

		std::vector<VkBuffer> 		_uniformBuffers;
		std::vector<VkDeviceMemory>	_uniformBuffersMemory;

		VkDescriptorPool 			_descriptorPool;
		std::vector<VkDescriptorSet>	_descriptorSets;

		//ALLOCATION DES COMMAND BUFFERS
		std::vector<VkCommandBuffer>	_commandBuffers;

		//SEMAPHORES
		std::vector<VkSemaphore>	_imageAvailableSemaphores;
		std::vector<VkSemaphore>	_renderFinishedSemaphores;
		std::vector<VkFence>		_inFlightFences;//les fences permettent au programme d'attendre l'exécution complète d'une opération. Nous allons créer une fence pour chaque frame
		std::vector<VkFence>		_imagesInFlight;
		size_t						_currentFrame = 0;

		//textures d'image (pixels -> texels)
		VkImage						_textureImage;
		VkDeviceMemory				_textureImageMemory;
		//reference a la vue pour la texture
		VkImageView					_textureImageView;
		VkSampler					_textureSampler;

};

#endif
