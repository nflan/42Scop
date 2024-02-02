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
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtx/hash.hpp>
#include <GLFW/glfw3.h>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/matrix_transform.hpp>

#include <chrono>
#include "tools.hpp"
#include "Mesh.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "PointLightSystem.hpp"
#include "RenderSystem.hpp"
#include "Descriptors.hpp"
#include "UniformBufferObject.hpp"
#include "KeyboardMovementController.hpp"
#include <filesystem>
#include <stdexcept>
#include <functional>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <cstdlib>

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
		void	run( void );
	private:
  		void	loadGameObjects();
		void	loadTextures();
		void	getText();
		void	getTextInDir();
		void	addMaterials();
		void	createTextureImage(const char *);
		void	createTextureImageView(Texture&);
		void	createTextureSampler(Texture&);
		void	createBuffers();
		void	createDescriptorSetLayout();
		void	createDescriptorSets();
		void	refreshDescriptorSets();
		void	createRenderSystems();
		void	generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void	transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

		// Mesh						_mesh;
		std::string					_file;
		std::string					_textFile;
		std::vector<std::string>	_textFiles;

		ft_Window							_window{WIDTH, HEIGHT, "FT_SCOP"};
		ft_Device							_device{_window};
		ft_Renderer							_renderer{_window, _device};
		std::unique_ptr<ft_DescriptorPool>	_globalPool{};
		std::unique_ptr<ft_DescriptorPool>	_globalPoolText{};
		std::unique_ptr<ft_DescriptorPool>	_changePoolText{};
		ft_GameObject::Map					_gameObjects;
		std::vector<Texture>				_loadedTextures;
		// std::map<std::string, ft_Material>	_materials;
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
