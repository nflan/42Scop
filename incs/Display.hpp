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
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtx/hash.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/matrix_transform.hpp>

#include <chrono>
#include "tools.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "PointLightSystem.hpp"
#include "RenderSystem.hpp"
#include "Descriptors.hpp"
#include "KeyboardMovementController.hpp"
#include <filesystem>
#include <stdexcept>
#include <functional>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <cstdlib>
#include <iostream>

const uint32_t	WIDTH = 800;
const uint32_t	HEIGHT = 600;

class Display
{
	public:
		Display( void );
		Display( const Display & );
		Display &	operator=( const Display& o );
		~Display( void );

		void	setFile(const char* file);
		void	setText(const char* file);
		bool	run( void );
	private:
  		void	loadGameObjects();
		bool	loadTextures();
		bool	getText();
		void	getTextInDir();
		void	addMaterials();
		bool	createTextureImage(const char *);
		void	createTextureImageView(Texture&);
		void	createTextureSampler(Texture&);
		void	createBuffers();
		void	createDescriptorSetLayout();
		void	createDescriptorSets();
		void	refreshDescriptorSets();
		void	createRenderSystems();
		bool	generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		bool	transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

		std::string							_file;
		std::string							_textFile;
		std::vector<std::string>			_textFiles;

		ft_Window							_window{WIDTH, HEIGHT, "FT_SCOP"};
		ft_Device							_device{_window};
		ft_Renderer							_renderer{_window, _device};
		std::unique_ptr<ft_DescriptorPool>	_globalPool{};
		std::unique_ptr<ft_DescriptorPool>	_globalPoolText{};
		std::unique_ptr<ft_DescriptorPool>	_changePoolText{};
		ft_GameObject::Map					_gameObjects;
		std::vector<Texture>				_loadedTextures;
		unsigned int						_currText;

		std::vector<std::unique_ptr<ft_Buffer>>		_buffers;

		std::vector<std::unique_ptr<RenderSystem>>		_renderSystems;
		std::vector<std::unique_ptr<PointLightSystem>>	_pointLightSystems;
		std::vector<std::unique_ptr<ft_DescriptorSetLayout>>	_globalDescriptorSetLayouts;

		//DESCRIPTORS
		VkDescriptorPool				_descriptorPool;
		VkDescriptorPool				_descriptorPoolWithoutTexture;
		std::vector<VkDescriptorSet>	_descriptorSets;
		std::vector<VkDescriptorSet>	_changeDescriptorSets;
		std::vector<VkDescriptorSet>	_descriptorSetsWithoutTexture;
};

#endif
