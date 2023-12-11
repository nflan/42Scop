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
#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
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
		void 	createDescriptorSets();
		void	createDescriptorSetLayout();
		void	createDescriptorPool();
		
		// bool	framebufferResized = false;
	private:
  		void	loadGameObjects();
		// Mesh						_mesh;
		const char*					_file;

		ft_Window					_window{WIDTH, HEIGHT, "FT_SCOP"};
		ft_Device					_device{_window};
		ft_Renderer					_renderer{_window, _device};
		std::unique_ptr<VkDescriptorPool>	_globalPool{};
		ft_GameObject::Map			_gameObjects;
		std::vector<Texture>		_loadedTextures;
		VkDescriptorSetLayout		_descriptor;
		VkDescriptorPool			_descriptorPool;
		std::vector<VkDescriptorSet>	_descriptorSets;
		std::vector<std::unique_ptr<ft_Buffer> > _uboBuffers;
		std::vector<std::shared_ptr<ft_Model> >		_models;

};

#endif
