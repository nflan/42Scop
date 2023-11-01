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
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include </home/nflan/bin/glm/glm/gtx/hash.hpp>
#include <GLFW/glfw3.h>
#include </home/nflan/bin/glm/glm/glm.hpp>
#include </home/nflan/bin/glm/glm/gtc/matrix_transform.hpp>

#include <chrono>
#include "tools.hpp"
#include "Mesh.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "RenderSystem.hpp"
#include "Descriptors.hpp"
#include "UniformBufferObject.hpp"
#include "KeyboardMovementController.hpp"
#include "Vertex.hpp"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <cstdlib>
#include <cstring>

const uint32_t	WIDTH = 800;
const uint32_t	HEIGHT = 600;

const std::string	MODEL_PATH = "models/viking_room.obj";
const std::string	TEXTURE_PATH = "textures/viking_room.png";

class Display
{
	public:
		Display( void );
		Display( const Display & );
		Display &	operator=( const Display& o );
		~Display( void );

		void	setFile(const char* file);
		void	run( void );
		bool	framebufferResized = false;
	private:
		// void	initVulkan( void ); // initImg
		// void	mainLoop( void ); // boucle
		// void	cleanup( void ); // clean
		// void	createInstance( void ); // create instance vulkan
		// bool	checkValidationLayerSupport( void ); //check layer (protections)
		// std::vector<const char*>	getRequiredExtensions(); // recup extensions debug
		// void	setupDebugMessenger( void ); // debug
		// void	populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		// void	pickPhysicalDevice(); // recuperations de toutes les cg
		// bool	isDeviceSuitable(VkPhysicalDevice device); // voir si la cg correspond a ce qu'on va faire
		// int		rateDeviceSuitability(VkPhysicalDevice device); // score en fonction des fonctionnalites de la cg
		// QueueFamilyIndices	findQueueFamilies(VkPhysicalDevice device); // trouver la famille de queue "graphique"
		// void	createLogicalDevice();
		// bool	checkDeviceExtensionSupport(VkPhysicalDevice device);
		// SwapChainSupportDetails	querySwapChainSupport(VkPhysicalDevice device);
		// VkSurfaceFormatKHR	chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		// VkPresentModeKHR	chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		// VkExtent2D	chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); // choisir le format de l'image, souvent taille de la fenetre, le width et height sont limites par cette derniere, mais s'ils sont en uint32 max, alors on peut choisir ce qu'on desire
		// void	createSwapChain(); //recup les informations des precedentes fonctions sur la swap
		// void	createImageViews();
		// void	createDescriptorSetLayout();
		// void	createGraphicsPipeline(); //https://vulkan-tutorial.com/fr/Dessiner_un_Display/Pipeline_graphique_basique/Introduction
		// VkShaderModule	createShaderModule(const std::vector<char>& code); //avant d'envoyer les infos des shaders dans la pipeline
		// void	createRenderPass();//https://vulkan-tutorial.com/fr/Dessiner_un_Display/Pipeline_graphique_basique/Render_pass
		// void	createFramebuffers();
		// void	createCommandPool();
		// void	createCommandBuffers();
		// void	drawFrame();
		// void	createSyncObjects();
		// void	recreateSwapChain();
		// void	cleanupSwapChain();
		// void	createVertexBuffer();
		// void	createIndexBuffer();
		// void	createUniformBuffers();
		// void 	updateUniformBuffer(uint32_t currentImage);
		// void	createDescriptorPool();
		// void	createDescriptorSets();
		// uint32_t	findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		// void 	createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		// void 	copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		//images mais a voir pour changer plus tard
		// void	createTextureImage();
		// void	createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		// VkCommandBuffer	beginSingleTimeCommands();
		// void	endSingleTimeCommands(VkCommandBuffer commandBuffer);
		// void	transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		// void	copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		// void	createTextureImageView();
		// VkImageView	createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipl);
		// void	createTextureSampler();
		// void	createDepthResources();
		// VkFormat	findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		// VkFormat	findDepthFormat();
		// bool	hasStencilComponent(VkFormat format);
		// void	loadModel();
		// void	generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		// VkSampleCountFlagBits	getMaxUsableSampleCount(VkSampleCountFlags requestedSampleCount);
		// void	createColorResources();

  		void loadGameObjects();
		// Mesh						_mesh;
		const char*					_file;

		ft_Window					_window{WIDTH, HEIGHT, "FT_SCOP"};
		ft_Device					_device{_window};
		ft_Renderer					_renderer{_window, _device};
		std::unique_ptr<ft_DescriptorPool>	_globalPool{};
		ft_GameObject::Map			_gameObjects;
		// VkSwapchainKHR				_swapChain;
		// std::vector<VkImage>		_swapChainImages; // images de la swap chain (auto del avec la swapchain)
		// VkFormat					_swapChainImageFormat;
		// VkExtent2D					_swapChainExtent;
		// std::vector<VkImageView>	_swapChainImageViews;
		//FRAMEBUFFERS
		// std::vector<VkFramebuffer>	_swapChainFramebuffers;

		//RENDER PASS
		// VkRenderPass				_renderPass;
		// VkDescriptorSetLayout 		_descriptorSetLayout;
		// VkPipelineLayout			_pipelineLayout;
		// VkPipeline					_graphicsPipeline;

		//VertexBuffers
		// std::vector<Vertex>			_vertices;
		// std::vector<uint32_t>		_indices;
		// VkBuffer					_vertexBuffer;
		// VkDeviceMemory				_vertexBufferMemory;
		// VkBuffer 					_indexBuffer;
		// VkDeviceMemory 				_indexBufferMemory;

		// std::vector<VkBuffer> 		_uniformBuffers;
		// std::vector<VkDeviceMemory>	_uniformBuffersMemory;

		// VkDescriptorPool 			_descriptorPool;
		// std::vector<VkDescriptorSet>	_descriptorSets;

		//ALLOCATION DES COMMAND BUFFERS
		// std::vector<VkCommandBuffer>	_commandBuffers;

		//SEMAPHORES
		// std::vector<VkSemaphore>	_imageAvailableSemaphores;
		// std::vector<VkSemaphore>	_renderFinishedSemaphores;
		// std::vector<VkFence>		_inFlightFences;//les fences permettent au programme d'attendre l'exécution complète d'une opération. Nous allons créer une fence pour chaque frame
		// std::vector<VkFence>		_imagesInFlight;
		// size_t						_currentFrame = 0;

		//textures d'image (pixels -> texels)
		// uint32_t					_mipLevels;
		// VkImage						_textureImage;
		// VkDeviceMemory				_textureImageMemory;
		//reference a la vue pour la texture
		// VkImageView					_textureImageView;
		// VkSampler					_textureSampler;

		//Image de profondeur et view
		// VkImage						_depthImage;
		// VkDeviceMemory				_depthImageMemory;
		// VkImageView					_depthImageView;

		// //Multisampling -> anti-aliasing
		// VkImage						_colorImage;
		// VkDeviceMemory				_colorImageMemory;
		// VkImageView					_colorImageView;

};

#endif
