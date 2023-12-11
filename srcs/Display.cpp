/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:10:26 by nflan             #+#    #+#             */
/*   Updated: 2023/10/27 12:46:37 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Display.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "/mnt/nfs/homes/nflan/sgoinfre/bin/stb/stb_image.h"

bool		QUIT = false;
bool		ROBJ = false;
short		WAY = 1;
float		ROTX = 0;
float		ROTY = ROTATION; //change in tools.hpp
float		ROTZ = 0;
const int	MAX_FRAMES_IN_FLIGHT = 2;

// namespace std {
//     template<> struct hash<ft_Model::Vertex> {
//         size_t operator()(ft_Model::Vertex const& vertex) const {
//             return ((hash<glm::vec3>()(vertex.pos) ^
//                    (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//                    (hash<glm::vec2>()(vertex.texCoord) << 1);
//         }
//     };
// }

static	std::vector<char> readFile(const std::string& filename) {
	std::ifstream	file(filename, std::ios::ate | std::ios::binary); //ate pour commencer a la fin / binary pour dire que c'est un binaire et pas recompiler

	if (!file.is_open())
		throw std::runtime_error(std::string {"Failed to open: "} + filename + "!");
	size_t	fileSize = (size_t) file.tellg(); // on a commence a la fin donc voir ou est le pointeur
	std::vector<char>	buffer(fileSize);

	file.seekg(0); // tout lire jusqu'au debut
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void	key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static_cast<void> (window);
	static_cast<void> (scancode);
	static_cast<void> (mods);
	if ((key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
		QUIT = true;
	else if ((key == GLFW_KEY_F) && action == GLFW_PRESS)
		WAY *= -1;
	else if ((key == GLFW_KEY_P) && action == GLFW_PRESS)
	{
		ROTY = ROTX = ROTZ = .0f;
	}
	else if ((key == GLFW_KEY_1) && action == GLFW_PRESS)
	{
		ROTY == .0f ? ROTY = ROTATION : ROTY = .0f;
	}
	else if ((key == GLFW_KEY_2) && action == GLFW_PRESS)
	{
		ROTX == .0f ? ROTX = ROTATION : ROTX = .0f;
	}
	else if ((key == GLFW_KEY_3) && action == GLFW_PRESS)
	{
		ROTZ == .0f ? ROTZ = ROTATION : ROTZ = .0f;
	}
	else if ((key == GLFW_KEY_4) && action == GLFW_PRESS)
	{
		ROBJ = true;
	}
	
}

Display::Display() {
	_globalPool =
		ft_DescriptorPool::Builder(_device)
		.setMaxSets(ft_SwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ft_SwapChain::MAX_FRAMES_IN_FLIGHT)
		.build();
}

Display::Display( const Display & o) {
	if (this != &o)
		*this = o;
	return ;
}

Display &	Display::operator=( const Display& o )
{
	if (this == &o)
		return (*this);
	*this = o;
	return (*this);
}

Display::~Display()
{
	for (Texture text : this->_loadedTextures)
	{
		vkDestroyImage(this->_device.device(), text._image, nullptr);
		vkFreeMemory(this->_device.device(), text._imageMemory, nullptr);
	}
}

void	Display::setFile(const char* file) { this->_file = file; }

void	Display::run()
{
    createTextureImage();
	loadGameObjects();

	std::vector<std::unique_ptr<ft_Buffer>> uboBuffers(ft_SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++)
	{
		uboBuffers[i] = std::make_unique<ft_Buffer>(
			this->_device,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	auto globalSetLayout =
		ft_DescriptorSetLayout::Builder(this->_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

	std::vector<VkDescriptorSet>	globalDescriptorSets(ft_SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++)
	{
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		DescriptorWriter(*globalSetLayout, *this->_globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	RenderSystem renderSystem{
		this->_device,
		this->_renderer.getSwapChainRenderPass(),
		globalSetLayout->getDescriptorSetLayout()};

	PointLightSystem pointLightSystem{
		this->_device,
		this->_renderer.getSwapChainRenderPass(),
		globalSetLayout->getDescriptorSetLayout()};

	ft_Camera camera{};

	auto viewerObject = ft_GameObject::createGameObject();

	viewerObject.transform.translation.z = -10.f;
	KeyboardMovementController cameraController(glm::vec3(0,0,-10));

	std::chrono::_V2::system_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	while (!this->_window.shouldClose() && !QUIT)
	{
		glfwSetKeyCallback(this->_window.getWindow(), key_callback);
		glfwPollEvents();

		std::chrono::_V2::system_clock::time_point newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ(this->_window.getWindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = this->_renderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

		if (auto commandBuffer = this->_renderer.beginFrame())
		{
			int frameIndex = this->_renderer.getFrameIndex();
			FrameInfo frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex],
				this->_gameObjects};

			// update
			GlobalUbo	ubo{};
			ubo.projection = camera.getProjection();
			ubo.view = camera.getView();
			ubo.inverseView = camera.getInverseView();
			pointLightSystem.update(frameInfo, ubo);
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			// render
			this->_renderer.beginSwapChainRenderPass(commandBuffer);
			
			renderSystem.renderGameObjects(frameInfo);
			pointLightSystem.render(frameInfo);

			this->_renderer.endSwapChainRenderPass(commandBuffer);
			this->_renderer.endFrame();
		}
	}

	vkDeviceWaitIdle(this->_device.device());
	// SimpleRenderSystem	simpleRenderSystem{this->_device.device(), this->_renderer.getSwapChainRenderPass()};
	// this->initVulkan();
	// this->mainLoop();
	// this->cleanup();
}

void Display::loadGameObjects()
{
  	std::shared_ptr<ft_Model> Model = ft_Model::createModelFromFile(this->_device, this->_file);
	ft_GameObject	gameObj = ft_GameObject::createGameObject();
	gameObj.model = Model;
	// gameObj.transform.scale = glm::vec3(0.5f);

	
	this->_gameObjects.emplace(gameObj.getId(), std::move(gameObj));

	std::vector<glm::vec3> lightColors{
		{1.f, 1.f, 1.f},
  	};

	for (int i = 0; i < lightColors.size(); i++)
	{
		ft_GameObject pointLight = ft_GameObject::makePointLight(0.2f);
		pointLight.color = lightColors[i];
		
		glm::mat4 rotateLight = glm::rotate(
			glm::mat4(1.5f),
			(i * glm::two_pi<float>()) / lightColors.size(),
			{0.f, -1.f, 0.f});
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
		this->_gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  	}
}

void	Display::loadTextures()
{
	createTexture("textures/viking_room.png");
}

void	Display::createTextureImage()
{
	Texture	text;
	int	texWidth, texHeight, texChannels;
    stbi_uc	*pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize	imageSize = texWidth * texHeight * 4;

	// this->_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;//+1 pour rajouter l'image originale

    if (!pixels)
	{
        throw std::runtime_error("échec du chargement d'une image!");
    }

	//Buffer intermediaire image
	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;
	this->_device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void	*data;
	vkMapMemory(this->_device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->_device.device(), stagingBufferMemory);

	stbi_image_free(pixels);

    this->_renderer.getSwapChain()->createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, text._image, text._imageMemory);

	this->transitionImageLayout(text._image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1); //this->_mipLevels
	this->_device.copyBufferToImage(stagingBuffer, text._image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
	this->transitionImageLayout(text._image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);//this->_mipLevels

	vkDestroyBuffer(this->_device.device(), stagingBuffer, nullptr);
    vkFreeMemory(this->_device.device(), stagingBufferMemory, nullptr);

	this->_loadedTextures.push_back(text);
	// generateMipmaps(text._image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, 1); //this->_mipLevels
}

void	Display::createTexture(char* file)
{

}

void	Display::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();

	VkImageMemoryBarrier	barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		throw std::invalid_argument("transition d'orgisation non supportée!");

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
    this->_device.endSingleTimeCommands(commandBuffer);
}

// static void	framebufferResizeCallback(GLFWwindow* window, int width, int height)
// {
// 	static_cast<void>(width);
// 	static_cast<void>(height);
// 	auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));
// 	app->framebufferResized = true;
// }


// void	Display::initVulkan( void )
// {
	// this->createInstance();
	// this->setupDebugMessenger();
	// this->_window.createWindowSurface(this->_instance, &this->_surface);
	// this->pickPhysicalDevice();
	// this->createLogicalDevice();
	// this->createSwapChain();
	// this->createImageViews();
	// this->createRenderPass();
	// this->createDescriptorSetLayout();
	// this->createGraphicsPipeline();
	// this->createCommandPool();
	// this->createColorResources();
	// this->createDepthResources();
	// this->createFramebuffers();
	// this->createTextureImage();
	// this->createTextureImageView();
	// this->createTextureSampler();
	// this->loadModel();
	// this->createVertexBuffer();
	// this->createIndexBuffer();
	// this->createUniformBuffers();
	// this->createDescriptorPool();
	// this->createDescriptorSets();
	// this->createCommandBuffers();
	// this->createSyncObjects();
// }

// void	Display::mainLoop( void )
// {
// 	auto	currentTime = std::chrono::high_resolution_clock::now();

// 	while (!this->_window.shouldClose() && !QUIT) {
// 		glfwSetKeyCallback(this->_window.getWindow(), key_callback);
// 		glfwPollEvents();
// 		auto	newTime = std::chrono::high_resolution_clock::now();
// 		float	frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
// 		currentTime = newTime;
// 		this->drawFrame();
// 	}

// 	vkDeviceWaitIdle(this->_device.device()); //attente de la fin des semaphores pour quitter
// }

// void	Display::recreateSwapChain()
// {
// 	// Quand fenetre minimisee, mise en pause du rendu
// 	int	width = 0;
// 	int	height = 0;
// 	while (width == 0 || height == 0)
// 	{
// 		glfwGetFramebufferSize(this->_window.getWindow(), &width, &height);
// 		glfwWaitEvents();
// 	}
// 	//

// 	vkDeviceWaitIdle(this->_device.device());

// 	this->cleanupSwapChain();

// 	// this->createSwapChain();
// 	// this->createImageViews();
// 	// this->createRenderPass();
// 	this->createGraphicsPipeline();
// 	// this->createColorResources();
// 	// this->createDepthResources();
// 	// this->createFramebuffers();
// 	this->createUniformBuffers();
// 	this->createDescriptorPool();
// 	this->createDescriptorSets();
// 	// this->createCommandBuffers();
// }

// void	Display::cleanupSwapChain()
// {
	// vkDestroyImageView(this->_device.device(), this->_depthImageView, nullptr);
    // vkDestroyImage(this->_device.device(), this->_depthImage, nullptr);
    // vkFreeMemory(this->_device.device(), this->_depthImageMemory, nullptr);

	// vkDestroyImageView(this->_device.device(), this->_colorImageView, nullptr);
    // vkDestroyImage(this->_device.device(), this->_colorImage, nullptr);
    // vkFreeMemory(this->_device.device(), this->_colorImageMemory, nullptr);

	// for (size_t i = 0; i < this->_swapChainFramebuffers.size(); i++) {
	// 	vkDestroyFramebuffer(this->_device.device(), this->_swapChainFramebuffers[i], nullptr);
	// }

	// vkFreeCommandBuffers(this->_device.device(), this->_device.getCommandPool(), static_cast<uint32_t>(this->_commandBuffers.size()), this->_commandBuffers.data());
    
	// for (size_t i = 0; i < this->_swapChainImages.size(); i++) {
    //     vkDestroyBuffer(this->_device.device(), this->_uniformBuffers[i], nullptr);
    //     vkFreeMemory(this->_device.device(), this->_uniformBuffersMemory[i], nullptr);
    // }

    // vkDestroyDescriptorPool(this->_device.device(), this->_descriptorPool, nullptr);
	// this->_descriptorPool = nullptr;
	
	// vkDestroyPipeline(this->_device.device(), this->_graphicsPipeline, nullptr);
	// vkDestroyPipelineLayout(this->_device.device(), this->_pipelineLayout, nullptr);
	// vkDestroyRenderPass(this->_device.device(), this->_renderPass, nullptr);

	// for (size_t i = 0; i < this->_swapChainImageViews.size(); i++)
	// 	vkDestroyImageView(this->_device.device(), this->_swapChainImageViews[i], nullptr);

	// vkDestroySwapchainKHR(this->_device.device(), this->_swapChain, nullptr);
// }

// void	Display::cleanup( void )
// {
	// this->cleanupSwapChain();

	// if (this->_descriptorPool != nullptr)
    // 	vkDestroyDescriptorPool(this->_device.device(), this->_descriptorPool, nullptr);

    // vkDestroySampler(this->_device.device(), this->_textureSampler, nullptr);
    // vkDestroyImageView(this->_device.device(), this->_textureImageView, nullptr);

    // vkDestroyImage(this->_device.device(), this->_textureImage, nullptr);
    // vkFreeMemory(this->_device.device(), this->_textureImageMemory, nullptr);

	// vkDestroyDescriptorSetLayout(this->_device.device(), this->_descriptorSetLayout, nullptr);

    // vkDestroyBuffer(this->_device.device(), this->_indexBuffer, nullptr);
    // vkFreeMemory(this->_device.device(), this->_indexBufferMemory, nullptr);

	// vkDestroyBuffer(this->_device.device(), this->_vertexBuffer, nullptr);
	// vkFreeMemory(this->_device.device(), this->_vertexBufferMemory, nullptr);

	// for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	// {
	// 	vkDestroySemaphore(this->_device.device(), this->_renderFinishedSemaphores[i], nullptr);
	// 	vkDestroySemaphore(this->_device.device(), this->_imageAvailableSemaphores[i], nullptr);
	// 	vkDestroyFence(this->_device.device(), this->_inFlightFences[i], nullptr);
	// }

	// vkDestroyCommandPool(this->_device.device(), this->_commandPool, nullptr);

	// vkDestroyDevice(this->_device.device(), nullptr);

	// if (enableValidationLayers)
		// DestroyDebugUtilsMessengerEXT(this->_instance, this->_debugMessenger, nullptr);

	// vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
	// vkDestroyInstance(this->_instance, nullptr);
// }

// void Display::createInstance()
// {
// 	if (enableValidationLayers && !checkValidationLayerSupport())
// 		throw std::runtime_error("validation layers requested, but not available!");

// 	VkApplicationInfo	appInfo{}; // informations optionnelles mais utiles pour optimiser
// 	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // expliciter le type
// 	appInfo.pApplicationName = "FT_SCOP"; // nom de l'app
// 	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // version
// 	appInfo.pEngineName = "No Engine"; // engine si utilise
// 	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
// 	appInfo.apiVersion = VK_API_VERSION_1_0;

// 	VkInstanceCreateInfo	createInfo{}; // structure permettant la création de l'instance. Celle-ci n'est pas optionnelle. -> informer le driver des extensions et des validation layers
// 	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
// 	createInfo.pApplicationInfo = &appInfo;

	// auto extensions = getRequiredExtensions();
// 	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
// 	createInfo.ppEnabledExtensionNames = extensions.data();

// 	VkDebugUtilsMessengerCreateInfoEXT	debugCreateInfo{};
// 	if (enableValidationLayers)
// 	{
// 		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
// 		createInfo.ppEnabledLayerNames = validationLayers.data();

// 		populateDebugMessengerCreateInfo(debugCreateInfo);
// 		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
// 	} else
// 	{
// 		createInfo.enabledLayerCount = 0;
// 		createInfo.pNext = nullptr;
// 	}

// 	if (vkCreateInstance(&createInfo, nullptr, &this->_instance) != VK_SUCCESS)
// 		throw std::runtime_error("failed to create instance!");
// 	//Pointeur sur une structure contenant l'information pour la création
// 	//Pointeur sur une fonction d'allocation que nous laisserons toujours nullptr
// 	//Pointeur sur une variable stockant une référence au nouvel objet
// }

// VkSampleCountFlagBits	Display::getMaxUsableSampleCount(VkSampleCountFlags requestedSampleCount)
// {
//     VkPhysicalDeviceProperties	physicalDeviceProperties;
//     vkGetPhysicalDeviceProperties(this->_physicalDevice, &physicalDeviceProperties);

//     VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

//     if (requestedSampleCount & counts & VK_SAMPLE_COUNT_64_BIT)
//         return VK_SAMPLE_COUNT_64_BIT;
//     if (requestedSampleCount & counts & VK_SAMPLE_COUNT_32_BIT)
//         return VK_SAMPLE_COUNT_32_BIT;
//     if (requestedSampleCount & counts & VK_SAMPLE_COUNT_16_BIT)
//         return VK_SAMPLE_COUNT_16_BIT;
//     if (requestedSampleCount & counts & VK_SAMPLE_COUNT_8_BIT)
//         return VK_SAMPLE_COUNT_8_BIT;
//     if (requestedSampleCount & counts & VK_SAMPLE_COUNT_4_BIT)
//         return VK_SAMPLE_COUNT_4_BIT;
//     if (requestedSampleCount & counts & VK_SAMPLE_COUNT_2_BIT)
//         return VK_SAMPLE_COUNT_2_BIT;

//     // If the requested sample count is not directly supported, find the nearest lower supported count
//     if (requestedSampleCount & VK_SAMPLE_COUNT_64_BIT)
//         return VK_SAMPLE_COUNT_64_BIT;
//     if (requestedSampleCount & VK_SAMPLE_COUNT_32_BIT)
//         return VK_SAMPLE_COUNT_32_BIT;
//     if (requestedSampleCount & VK_SAMPLE_COUNT_16_BIT)
//         return VK_SAMPLE_COUNT_16_BIT;
//     if (requestedSampleCount & VK_SAMPLE_COUNT_8_BIT)
//         return VK_SAMPLE_COUNT_8_BIT;
//     if (requestedSampleCount & VK_SAMPLE_COUNT_4_BIT)
//         return VK_SAMPLE_COUNT_4_BIT;
//     if (requestedSampleCount & VK_SAMPLE_COUNT_2_BIT)
//         return VK_SAMPLE_COUNT_2_BIT;

//     return VK_SAMPLE_COUNT_1_BIT;
// }

// void	Display::createColorResources()
// {
// 	VkFormat	colorFormat = this->_swapChainImageFormat;

//     createImage(this->_swapChainExtent.width, this->_swapChainExtent.height, 1, this->_device.getMsaaSamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_colorImage, this->_colorImageMemory);
//     this->_colorImageView = createImageView(this->_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
// }

// void	Display::loadModel()
// {
	// tinyobj::attrib_t	attrib;
    // std::vector<tinyobj::shape_t>	shapes;
    // std::vector<tinyobj::material_t>	materials;
    // std::string	warn, err;

    // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
    //     throw std::runtime_error(warn + err);
	// this->_vertices.clear();
	// std::unordered_map<Vertex, uint32_t> uniqueVertices;

	// for (uint32_t i = 0; i < this->_mesh.getFaceIndex().size(); i++)
	// {
	// 	Vertex vertex{};

	// 	vertex.pos = this->_mesh.getMeshVertices()[i];
	// 	if (this->_mesh.getTexCoord().size() > i)
	// 		vertex.texCoord = this->_mesh.getTexCoord()[i];
	// 	else
	// 		vertex.texCoord = {0.0f, 0.0f};
	// 	vertex.color = {static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX)};

		// if (uniqueVertices.count(vertex) == 0)
		// {
		// 	uniqueVertices[vertex] = static_cast<uint32_t>(this->_vertices.size());
		// 	this->_vertices.push_back(vertex);
		// }
// 		this->_vertices.push_back(vertex);
// 		this->_indices.push_back(this->_indices.size());
		// this->_indices.push_back(uniqueVertices[vertex]);
// 	}
// }

// void	Display::createTextureSampler()
// {
// 	VkSamplerCreateInfo	samplerInfo{}; //permet d'indiquer les filtres et les transformations à appliquer.
// 	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
// 	samplerInfo.magFilter = VK_FILTER_LINEAR; //interpoler texels magnifies
// 	samplerInfo.minFilter = VK_FILTER_LINEAR; //interpoler texels minifies

// 	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
// 	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	/*
	VK_SAMPLER_ADDRESS_MODE_REPEAT : répète le texture
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT : répète en inversant les coordonnées pour réaliser un effet miroir
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : prend la couleur du pixel de bordure le plus proche
	VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : prend la couleur de l'opposé du plus proche côté de l'image
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER : utilise une couleur fixée
	*/
	// samplerInfo.anisotropyEnable = VK_TRUE; ///on pourrait le desactiver si le mec n'a pas une cg qui peut le faire
	// samplerInfo.maxAnisotropy = 16.0f; // et passer ca a 1.0f si le mec n'a pas une cg pour le faire
	// samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //si l'image est plus petite que la fenetre, couleur du reste mais que black white or transparent
	// samplerInfo.unnormalizedCoordinates = VK_FALSE;
	//Le champ unnomalizedCoordinates indique le système de coordonnées que vous voulez utiliser pour accéder aux texels de l'image. Avec VK_TRUE, vous pouvez utiliser des coordonnées dans [0, texWidth) et [0, texHeight). Sinon, les valeurs sont accédées avec des coordonnées dans [0, 1). Dans la plupart des cas les coordonnées sont utilisées normalisées car cela permet d'utiliser un même shader pour des textures de résolution différentes.
	// samplerInfo.compareEnable = VK_FALSE;
	// samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	// samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	// samplerInfo.mipLodBias = 0.0f;//Lod -> Level of Details
	// samplerInfo.minLod = 0.0f; //static_cast<float>(this->_mipLevels / 2);//minimum de details
    // samplerInfo.maxLod = static_cast<float>(this->_mipLevels);//maximum de details

	// if (vkCreateSampler(this->_device.device(), &samplerInfo, nullptr, &this->_textureSampler) != VK_SUCCESS)
    //     throw std::runtime_error("échec de la creation d'un sampler!");
	// le sampler n'est pas lie a une image ! Il est independant et peut du coup etre efficace tout le long du programme. par contre si on veut changer la facon d'afficher, faut ptete le detruire et le refaire ?
// }

// VkFormat	Display::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
// {
// 	for (VkFormat format : candidates)
// 	{
//     	VkFormatProperties	props;
//     	vkGetPhysicalDeviceFormatProperties(this->_physicalDevice, format, &props);
// 		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
// 			return format;
// 		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
// 			return format;
// 	}
//     throw std::runtime_error("failed to find supported format!");
// }

// VkFormat	Display::findDepthFormat()
// {
// 	return findSupportedFormat(
//         {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
//         VK_IMAGE_TILING_OPTIMAL,
//         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
//     );
// }

// bool	Display::hasStencilComponent(VkFormat format)
// {
//     return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
// }

// void	Display::createDepthResources()
// {
// 	VkFormat	depthFormat = this->_device.findDepthFormat();

// 	this->createImage(this->_swapChainExtent.width, this->_swapChainExtent.height, 1, this->_device.getMsaaSamples(), depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_depthImage, this->_depthImageMemory);

// 	this->_depthImageView = this->createImageView(this->_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
// }

// void	Display::createTextureImage()
// {
// 	int	texWidth, texHeight, texChannels;
//     stbi_uc	*pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//     VkDeviceSize	imageSize = texWidth * texHeight * 4;

// 	this->_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;//+1 pour rajouter l'image originale

//     if (!pixels)
// 	{
//         throw std::runtime_error("échec du chargement d'une image!");
//     }

// 	//Buffer intermediaire image
// 	VkBuffer		stagingBuffer;
// 	VkDeviceMemory	stagingBufferMemory;
// 	this->_device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

// 	void	*data;
// 	vkMapMemory(this->_device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
//     memcpy(data, pixels, static_cast<size_t>(imageSize));
// 	vkUnmapMemory(this->_device.device(), stagingBufferMemory);

// 	stbi_image_free(pixels);

//     this->createImage(texWidth, texHeight, this->_mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_textureImage, this->_textureImageMemory);

// 	this->transitionImageLayout(this->_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, this->_mipLevels);
// 	this->_device.copyBufferToImage(stagingBuffer, this->_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
// 	// this->transitionImageLayout(this->_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, this->_mipLevels);

// 	vkDestroyBuffer(this->_device.device(), stagingBuffer, nullptr);
//     vkFreeMemory(this->_device.device(), stagingBufferMemory, nullptr);

// 	generateMipmaps(this->_textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, this->_mipLevels);
// }

// void	Display::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
// {
// 	//Verifier si l'image supporte le filtrage lineaire
// 	VkFormatProperties	formatProperties;
//     vkGetPhysicalDeviceFormatProperties(this->_device.getPhysicalDevice(), imageFormat, &formatProperties);

// 	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
//     	throw std::runtime_error("le format de l'image texture ne supporte pas le filtrage lineaire!");

// 	VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();

//     VkImageMemoryBarrier barrier{};
//     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//     barrier.image = image;
//     barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//     barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     barrier.subresourceRange.baseArrayLayer = 0;
//     barrier.subresourceRange.layerCount = 1;
//     barrier.subresourceRange.levelCount = 1;

// 	int32_t mipWidth = texWidth;
// 	int32_t mipHeight = texHeight;

// 	for (uint32_t i = 1; i < mipLevels; i++)
// 	{
// 		barrier.subresourceRange.baseMipLevel = i - 1;
// 		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// 		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

// 		vkCmdPipelineBarrier(commandBuffer,
// 			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
// 			0, nullptr,
// 			0, nullptr,
// 			1, &barrier);
		
// 		VkImageBlit blit{};
// 		blit.srcOffsets[0] = { 0, 0, 0 };
// 		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
// 		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 		blit.srcSubresource.mipLevel = i - 1;
// 		blit.srcSubresource.baseArrayLayer = 0;
// 		blit.srcSubresource.layerCount = 1;
// 		blit.dstOffsets[0] = { 0, 0, 0 };
// 		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
// 		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 		blit.dstSubresource.mipLevel = i;
// 		blit.dstSubresource.baseArrayLayer = 0;
// 		blit.dstSubresource.layerCount = 1;
		
// 		vkCmdBlitImage(commandBuffer,
// 			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
// 			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,//queue graphique obligatoire
// 			1, &blit,
// 			VK_FILTER_LINEAR);

// 		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
// 		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
// 		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 		vkCmdPipelineBarrier(commandBuffer,
// 			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// 			0, nullptr,
// 			0, nullptr,
// 			1, &barrier);

// 		if (mipWidth > 1)
// 			mipWidth /= 2;
// 		if (mipHeight > 1)
// 			mipHeight /= 2;
// 	}
// 	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
// 	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
// 	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
// 	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 	vkCmdPipelineBarrier(commandBuffer,
// 		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
// 		0, nullptr,
// 		0, nullptr,
// 		1, &barrier);

//     this->_device.endSingleTimeCommands(commandBuffer);
// }

// void	Display::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
// {
// 	VkImageCreateInfo	imageInfo{};
// 	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
// 	imageInfo.imageType = VK_IMAGE_TYPE_2D;
// 	//1D = utilisé comme des tableaux ou des gradients
// 	//2D = majoritairement utilisés comme textures
// 	//3D = utilisées pour stocker des voxels par exemple
//     imageInfo.extent.width = width;
//     imageInfo.extent.height = height;
// 	imageInfo.extent.depth = 1;
// 	imageInfo.mipLevels = mipLevels;
// 	imageInfo.arrayLayers = 1;
// 	imageInfo.format = format; //meme que dans celles dans le buffer
// 	imageInfo.tiling = tiling;
// 	//VK_IMAGE_TILING_LINEAR : les texels sont organisés ligne par ligne
// 	//VK_IMAGE_TILING_OPTIMAL : les texels sont organisés de la manière la plus optimale pour l'implémentation
// 	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	//VK_IMAGE_LAYOUT_UNDEFINED : inutilisable par le GPU, son contenu sera éliminé à la première transition
// 	//VK_IMAGE_LAYOUT_PREINITIALIZED : inutilisable par le GPU, mais la première transition conservera les texels
// 	imageInfo.usage = usage;
// 	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//     imageInfo.samples = numSamples;
// 	imageInfo.flags = 0; // Optionnel
// 	//Ces image étendues sont des images dont seule une partie est stockée dans la mémoire. Voici une exemple d'utilisation : si vous utilisiez une image 3D pour représenter un terrain à l'aide de voxels, vous pourriez utiliser cette fonctionnalité pour éviter d'utiliser de la mémoire qui au final ne contiendrait que de l'air. Nous ne verrons pas cette fonctionnalité dans ce tutoriel, donnez à flags la valeur 0.

//     if (vkCreateImage(this->_device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
//         throw std::runtime_error("echec de la creation d'une image!");

//     VkMemoryRequirements	memRequirements;
//     vkGetImageMemoryRequirements(this->_device.device(), image, &memRequirements);

//     VkMemoryAllocateInfo	allocInfo{};
//     allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//     allocInfo.allocationSize = memRequirements.size;
//     allocInfo.memoryTypeIndex = this->_device.findMemoryType(memRequirements.memoryTypeBits, properties);

//     if (vkAllocateMemory(this->_device.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
//         throw std::runtime_error("echec de l'allocation de la memoire d'une image!");

//     vkBindImageMemory(this->_device.device(), image, imageMemory, 0);
// }

// VkCommandBuffer	Display::beginSingleTimeCommands()
// {
//     VkCommandBufferAllocateInfo	allocInfo{};
//     allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//     allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//     allocInfo.commandPool = this->_commandPool;
//     allocInfo.commandBufferCount = 1;

//     VkCommandBuffer commandBuffer;
//     vkAllocateCommandBuffers(this->_device.device(), &allocInfo, &commandBuffer);

//     VkCommandBufferBeginInfo beginInfo{};
//     beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//     beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

//     vkBeginCommandBuffer(commandBuffer, &beginInfo);

//     return	commandBuffer;
// }

// void	Display::endSingleTimeCommands(VkCommandBuffer commandBuffer)
// {
//     vkEndCommandBuffer(commandBuffer);

//     VkSubmitInfo	submitInfo{};
//     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//     submitInfo.commandBufferCount = 1;
//     submitInfo.pCommandBuffers = &commandBuffer;

//     vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//     vkQueueWaitIdle(this->_graphicsQueue);

//     vkFreeCommandBuffers(this->_device.device(), this->_commandPool, 1, &commandBuffer);
// }

// void 	Display::createDescriptorSets()
// {
	// std::vector<VkDescriptorSetLayout> layouts(this->_swapChainImages.size(), this->_descriptorSetLayout);
	// // VkDescriptorSetAllocateInfo allocInfo{};
	// // allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	// // allocInfo.descriptorPool = this->_descriptorPool;
	// // allocInfo.descriptorSetCount = static_cast<uint32_t>(this->_swapChainImages.size());
	// // allocInfo.pSetLayouts = layouts.data();

	// this->_descriptorSets.resize(this->_swapChainImages.size());
	// if (vkAllocateDescriptorSets(this->_device.device(), &allocInfo, this->_descriptorSets.data()) != VK_SUCCESS)
    // 	throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
	// for (size_t i = 0; i < this->_swapChainImages.size(); i++) {
    // 	VkDescriptorBufferInfo 	bufferInfo{};
    // 	bufferInfo.buffer = this->_uniformBuffers[i];
    // 	bufferInfo.offset = 0;
    // 	bufferInfo.range = sizeof(UniformBufferObject);

	// 	VkDescriptorImageInfo imageInfo{};
	// 	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// 	imageInfo.imageView = this->_textureImageView;
	// 	imageInfo.sampler = this->_textureSampler;

	// 	std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

	// 	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// 	descriptorWrites[0].dstSet = this->_descriptorSets[i];
	// 	descriptorWrites[0].dstBinding = 0;
	// 	descriptorWrites[0].dstArrayElement = 0;
	// 	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// 	descriptorWrites[0].descriptorCount = 1;
	// 	descriptorWrites[0].pBufferInfo = &bufferInfo;

	// 	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// 	descriptorWrites[1].dstSet = this->_descriptorSets[i];
	// 	descriptorWrites[1].dstBinding = 1;
	// 	descriptorWrites[1].dstArrayElement = 0;
	// 	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// 	descriptorWrites[1].descriptorCount = 1;
	// 	descriptorWrites[1].pImageInfo = &imageInfo;

	// 	vkUpdateDescriptorSets(this->_device.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	// }
// }

// void 	Display::createDescriptorPool()
// {
// 	std::array<VkDescriptorPoolSize, 2> poolSizes{};
// 	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
// 	poolSizes[0].descriptorCount = static_cast<uint32_t>(this->_swapChainImages.size());
// 	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	poolSizes[1].descriptorCount = static_cast<uint32_t>(this->_swapChainImages.size());
	
// 	VkDescriptorPoolCreateInfo poolInfo{};
// 	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
// 	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
// 	poolInfo.pPoolSizes = poolSizes.data();
// 	poolInfo.maxSets = static_cast<uint32_t>(this->_swapChainImages.size());

// 	if (vkCreateDescriptorPool(this->_device.device(), &poolInfo, nullptr, &this->_descriptorPool) != VK_SUCCESS)
//     	throw std::runtime_error("echec de la creation de la pool de descripteurs!");
// }

// void	Display::createUniformBuffers()
// {
	// VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // this->_uniformBuffers.resize(this->_swapChainImages.size());
    // this->_uniformBuffersMemory.resize(this->_swapChainImages.size());

    // for (size_t i = 0; i < this->_swapChainImages.size(); i++) {
    //     this->_device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->_uniformBuffers[i], this->_uniformBuffersMemory[i]);
    // }
// }

// void	Display::createDescriptorSetLayout()
// {
// 	VkDescriptorSetLayoutBinding uboLayoutBinding{};
//     uboLayoutBinding.binding = 0;
//     uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     uboLayoutBinding.descriptorCount = 1;
// 	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
// 	uboLayoutBinding.pImmutableSamplers = nullptr; // Optionnel

// 	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
// 	samplerLayoutBinding.binding = 1;
// 	samplerLayoutBinding.descriptorCount = 1;
// 	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
// 	samplerLayoutBinding.pImmutableSamplers = nullptr;
// 	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

// 	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

// 	VkDescriptorSetLayoutCreateInfo layoutInfo{};
// 	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
// 	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
// 	layoutInfo.pBindings = bindings.data();

// 	if (vkCreateDescriptorSetLayout(this->_device.device(), &layoutInfo, nullptr, &this->_descriptorSetLayout) != VK_SUCCESS) {
//     	throw std::runtime_error("echec de la creation d'un set de descripteurs!");
// 	}
// }

// void 	Display::createIndexBuffer()
// {
//     VkDeviceSize 	bufferSize = sizeof(this->_indices[0]) * this->_indices.size();

//     VkBuffer 	stagingBuffer;
//     VkDeviceMemory 	stagingBufferMemory;
//     this->_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

//     void	*data;
//     vkMapMemory(this->_device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
//     memcpy(data, this->_indices.data(), (size_t) bufferSize);
//     vkUnmapMemory(this->_device.device(), stagingBufferMemory);

//     this->_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_indexBuffer, this->_indexBufferMemory);

//     this->_device.copyBuffer(stagingBuffer, this->_indexBuffer, bufferSize);

//     vkDestroyBuffer(this->_device.device(), stagingBuffer, nullptr);
//     vkFreeMemory(this->_device.device(), stagingBufferMemory, nullptr);
// }

// void	Display::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
// {
// 	VkBufferCreateInfo bufferInfo{};
// 	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
// 	bufferInfo.size = size;
// 	bufferInfo.usage = usage;
// 	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

// 	if (vkCreateBuffer(this->_device.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create vertex buffer!");
// 	}

// 	VkMemoryRequirements memRequirements;
// 	vkGetBufferMemoryRequirements(this->_device.device(), buffer, &memRequirements);

// 	VkMemoryAllocateInfo allocInfo{};
// 	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
// 	allocInfo.allocationSize = memRequirements.size;
// 	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

// 	if (vkAllocateMemory(this->_device.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
//         throw std::runtime_error("failed to allocate vertex buffer memory!");

// 	vkBindBufferMemory(this->_device.device(), buffer, bufferMemory, 0);
// 	/*
// 	 * Le quatrième indique le décalage entre le début de la mémoire et le début du buffer. Nous avons alloué cette mémoire spécialement pour ce buffer, nous pouvons donc mettre 0. Si vous décidez d'allouer un grand espace mémoire pour y mettre plusieurs buffers, sachez qu'il faut que ce nombre soit divisible par memRequirements.alignement. Notez que cette stratégie est la manière recommandée de gérer la mémoire des GPUs (https://developer.nvidia.com/vulkan-memory-management)
// 	 */
// }

// uint32_t	Display::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
// {
// 	VkPhysicalDeviceMemoryProperties	memProperties;
// 	//on recupere les types de memoire de la CG
// 	//on ne va pas le faire mais on peut choisir celle qui est la plus performante !
// 	vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memProperties);

// 	//on cherche un type de memoire qui correspond au buffer
// 	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
// 		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
// 			return i;
//   	throw std::runtime_error("failed to find suitable memory type!");
// }

// void 	Display::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
// {
//     VkCommandBuffer	commandBuffer = beginSingleTimeCommands();

//     VkBufferCopy	copyRegion{};
//     copyRegion.size = size;
//     vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

//     endSingleTimeCommands(commandBuffer);
// }

// void	Display::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
// {
//     VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();

// 	VkImageMemoryBarrier	barrier{};
// 	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
// 	barrier.oldLayout = oldLayout;
// 	barrier.newLayout = newLayout;
// 	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
// 	barrier.image = image;
// 	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	barrier.subresourceRange.baseMipLevel = 0;
// 	barrier.subresourceRange.levelCount = mipLevels;
// 	barrier.subresourceRange.baseArrayLayer = 0;
// 	barrier.subresourceRange.layerCount = 1;

// 	VkPipelineStageFlags sourceStage;
// 	VkPipelineStageFlags destinationStage;

// 	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
// 	{
// 		barrier.srcAccessMask = 0;
// 		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

// 		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
// 		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 	}
// 	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
// 	{
// 		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

// 		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
// 		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
// 	}
// 	else
// 		throw std::invalid_argument("transition d'orgisation non supportée!");

// 	vkCmdPipelineBarrier(
// 		commandBuffer,
// 		sourceStage, destinationStage,
// 		0,
// 		0, nullptr,
// 		0, nullptr,
// 		1, &barrier
// 	);
//     this->_device.endSingleTimeCommands(commandBuffer);
// }

// void	Display::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
// {
//     VkCommandBuffer commandBuffer = beginSingleTimeCommands();

// 	VkBufferImageCopy region{};
// 	region.bufferOffset = 0;
// 	region.bufferRowLength = 0;
// 	region.bufferImageHeight = 0;

// 	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	region.imageSubresource.mipLevel = 0;
// 	region.imageSubresource.baseArrayLayer = 0;
// 	region.imageSubresource.layerCount = 1;

// 	region.imageOffset = {0, 0, 0};
// 	region.imageExtent = {
// 		width,
// 		height,
// 		1
// 	};

// 	vkCmdCopyBufferToImage(
// 		commandBuffer,
// 		buffer,
// 		image,
// 		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
// 		1,
// 		&region
// 	);

// 	endSingleTimeCommands(commandBuffer);
// }

// void	Display::createVertexBuffer()
// {
// 	VkDeviceSize	bufferSize = sizeof(this->_vertices[0]) * this->_vertices.size();

// 	VkBuffer	stagingBuffer;
// 	VkDeviceMemory	stagingBufferMemory;
// 	this->_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

// 	void* data;
// 	/*
// 	 * Cette fonction nous permet d'accéder à une région spécifique d'une ressource. Nous devons pour cela indiquer un décalage et une taille. Nous mettons ici respectivement 0 et bufferInfo.size. Il est également possible de fournir la valeur VK_WHOLE_SIZE pour mapper d'un coup toute la ressource. L'avant-dernier paramètre est un champ de bits pour l'instant non implémenté par Vulkan. Il est impératif de la laisser à 0. Enfin, le dernier paramètre permet de fournir un pointeur vers la mémoire ainsi mappée.
// 	 */
// 	vkMapMemory(this->_device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
// 	memcpy(data, this->_vertices.data(), (size_t) bufferSize);
// 	vkUnmapMemory(this->_device.device(), stagingBufferMemory);
// 	/*
// 	 * Vous pouvez maintenant utiliser memcpy pour copier les this->_vertices dans la mémoire, puis démapper le buffer à l'aide de vkUnmapMemory. Malheureusement le driver peut décider de cacher les données avant de les copier dans le buffer. Il est aussi possible que les données soient copiées mais que ce changement ne soit pas visible immédiatement. Il y a deux manières de régler ce problème :

// 	 Utiliser une pile de mémoire cohérente avec la RAM, ce qui est indiqué par VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
// 	 Appeler vkFlushMappedMemoryRanges après avoir copié les données, puis appeler vkInvalidateMappedMemory avant d'accéder à la mémoire
// 	 */
// 	this->_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_vertexBuffer, this->_vertexBufferMemory);

// 	this->_device.copyBuffer(stagingBuffer, this->_vertexBuffer, bufferSize);
// 	vkDestroyBuffer(this->_device.device(), stagingBuffer, nullptr);
// 	vkFreeMemory(this->_device.device(), stagingBufferMemory, nullptr);
// }

// void	Display::createSyncObjects()
// {
// 	this->_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
// 	this->_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
// 	this->_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
// 	this->_imagesInFlight.resize(this->_swapChainImages.size(), VK_NULL_HANDLE);

// 	VkSemaphoreCreateInfo semaphoreInfo{};
// 	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

// 	VkFenceCreateInfo fenceInfo{};
// 	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
// 	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

// 	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
// 	{
// 		if (vkCreateSemaphore(this->_device.device(), &semaphoreInfo, nullptr, &this->_imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(this->_device.device(), &semaphoreInfo, nullptr, &this->_renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(this->_device.device(), &fenceInfo, nullptr, &this->_inFlightFences[i]) != VK_SUCCESS)

// 			throw std::runtime_error("échec de la création des objets de synchronisation pour une frame!");
// 	}
// }

// void	Display::drawFrame()
// {
	/*
	   1)Acquérir une image depuis la swap chain
	   2)Exécuter le command buffer correspondant au framebuffer dont l'attachement est l'image obtenue
	   3)Retourner l'image à la swap chain pour présentation
	   */
	// vkWaitForFences(this->_device.device(), 1, &this->_inFlightFences[this->_currentFrame], VK_TRUE, UINT64_MAX);//VK_TRUE permet d'attendre que TOUTES les fences soient good

// 	uint32_t	imageIndex;
//     VkResult	result = vkAcquireNextImageKHR(this->_device.device(), this->_swapChain, UINT64_MAX, this->_imageAvailableSemaphores[this->_currentFrame], VK_NULL_HANDLE, &imageIndex);
// 	if (result == VK_ERROR_OUT_OF_DATE_KHR)
// 	{
// 		recreateSwapChain();
// 		return;
// 	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
// 		throw std::runtime_error("échec de la présentation d'une image à la swap chain!");
// 	}

// 	// Vérifier si une frame précédente est en train d'utiliser cette image (il y a une fence à attendre)
// 	if (this->_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
// 		vkWaitForFences(this->_device.device(), 1, &this->_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
// 	// Marque l'image comme étant à nouveau utilisée par cette frame
// 	this->_imagesInFlight[imageIndex] = this->_inFlightFences[this->_currentFrame];

// 	//mise a jour des donnes uniformes
//     updateUniformBuffer(imageIndex);

// 	//Envoi du command buffer
// 	VkSubmitInfo submitInfo{};
// 	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

// 	VkSemaphore waitSemaphores[] = {this->_imageAvailableSemaphores[this->_currentFrame]};
// 	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
// 	submitInfo.waitSemaphoreCount = 1;
// 	submitInfo.pWaitSemaphores = waitSemaphores;
// 	submitInfo.pWaitDstStageMask = waitStages;

// 	submitInfo.commandBufferCount = 1;
// 	submitInfo.pCommandBuffers = &this->_commandBuffers[imageIndex];

// 	VkSemaphore signalSemaphores[] = {this->_renderFinishedSemaphores[this->_currentFrame]};
// 	submitInfo.signalSemaphoreCount = 1;
// 	submitInfo.pSignalSemaphores = signalSemaphores;

// 	vkResetFences(this->_device.device(), 1, &this->_inFlightFences[this->_currentFrame]);//obliger de rester a la main
// 	if (vkQueueSubmit(this->_device.getGraphicQueue(), 1, &submitInfo, this->_inFlightFences[this->_currentFrame]) != VK_SUCCESS)
// 		throw std::runtime_error("échec de l'envoi d'un command buffer!");

// 	//Presentation
// 	VkPresentInfoKHR presentInfo{};
// 	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
// 	presentInfo.waitSemaphoreCount = 1;
// 	presentInfo.pWaitSemaphores = signalSemaphores;

// 	VkSwapchainKHR swapChains[] = {this->_swapChain};
// 	presentInfo.swapchainCount = 1;
// 	presentInfo.pSwapchains = swapChains;
// 	presentInfo.pImageIndices = &imageIndex;
// 	presentInfo.pResults = nullptr; // Optionnel

// 	//	VkResult	result = vkAcquireNextImageKHR(this->_device.device(), this->_swapChain, UINT64_MAX, this->_imageAvailableSemaphores[this->_currentFrame], VK_NULL_HANDLE, &imageIndex);

// 	result = vkQueuePresentKHR(this->_device.getPresentQueue(), &presentInfo);

// 	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->framebufferResized)
// 	{
// 		this->framebufferResized = false;
// 		recreateSwapChain();
// 	}
// 	else if (result != VK_SUCCESS)
// 		throw std::runtime_error("échec de la présentation d'une image à la swap chain!");

// 	this->_currentFrame = (this->_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
// }

// void	Display::updateUniformBuffer(uint32_t currentImage)
// {
// 	static auto 	startTime = std::chrono::high_resolution_clock::now();

//     auto 	currentTime = std::chrono::high_resolution_clock::now();
//     float 	time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

// 	UniformBufferObject ubo{};
// 	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
// 	ubo.view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
// 	ubo.proj = glm::perspective(glm::radians(45.0f), this->_swapChainExtent.width / (float) this->_swapChainExtent.height, 0.1f, 20.0f);
// 	ubo.proj[1][1] *= -1; //glm fait pour opengl donc inverse x y

// 	void*	data;
// 	vkMapMemory(this->_device.device(), this->_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
//     	memcpy(data, &ubo, sizeof(ubo));
// 	vkUnmapMemory(this->_device.device(), this->_uniformBuffersMemory[currentImage]);
// }

// void	Display::createCommandBuffers()
// {
// 	this->_commandBuffers.resize(this->_swapChainFramebuffers.size());

// 	VkCommandBufferAllocateInfo	allocInfo{};
// 	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
// 	allocInfo.commandPool = this->_device.getCommandPool();
// 	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
// 	//VK_COMMAND_BUFFER_LEVEL_PRIMARY : peut être envoyé à une queue pour y être exécuté mais ne peut être appelé par d'autres command buffers
// 	//VK_COMMAND_BUFFER_LEVEL_SECONDARY : ne peut pas être directement émis à une queue mais peut être appelé par un autre command buffer
// 	allocInfo.commandBufferCount = (uint32_t) this->_commandBuffers.size();

// 	if (vkAllocateCommandBuffers(this->_device.device(), &allocInfo, this->_commandBuffers.data()) != VK_SUCCESS)
// 		throw std::runtime_error("échec de l'allocation de command buffers!");

// 	//Début de l'enregistrement des commandes
// 	std::array<VkClearValue, 2>	clearValues{};
// 	clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
// 	clearValues[1].depthStencil = {1.0f, 0};//Dans vulkan, 0.0 correspond au plan near et 1.0 far.
// 	for (size_t i = 0; i < this->_commandBuffers.size(); i++)
// 	{
// 		VkCommandBufferBeginInfo	beginInfo{};
// 		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
// 		beginInfo.flags = 0; // Optionnel
// 		beginInfo.pInheritanceInfo = nullptr; // Optionel

// 		if (vkBeginCommandBuffer(this->_commandBuffers[i], &beginInfo) != VK_SUCCESS)
// 			throw std::runtime_error("erreur au début de l'enregistrement d'un command buffer!");
// 		//Commencer une render pass
// 		VkRenderPassBeginInfo renderPassInfo{};
// 		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
// 		renderPassInfo.renderPass = this->_renderPass;
// 		renderPassInfo.framebuffer = this->_swapChainFramebuffers[i];
// 		renderPassInfo.renderArea.offset = {0, 0};
// 		renderPassInfo.renderArea.extent = this->_swapChainExtent;
// 		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
// 		renderPassInfo.pClearValues = clearValues.data();
// 		vkCmdBeginRenderPass(this->_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
// 		//VK_SUBPASS_CONTENTS_INLINE : les commandes de la render pass seront inclues directement dans le command buffer (qui est donc primaire)
// 		//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFER : les commandes de la render pass seront fournies par un ou plusieurs command buffers secondaires

// 		//Commandes d'affichage basiques
// 		vkCmdBindPipeline(this->_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->_graphicsPipeline);
// 		VkBuffer	vertexBuffers[] = {this->_vertexBuffer};
// 		VkDeviceSize	offsets[] = {0};
// 		vkCmdBindVertexBuffers(this->_commandBuffers[i], 0, 1, vertexBuffers, offsets);

// 		vkCmdBindIndexBuffer(this->_commandBuffers[i], this->_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		
// 		vkCmdBindDescriptorSets(this->_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->_pipelineLayout, 0, 1, &this->_descriptorSets[i], 0, nullptr);
// 		vkCmdDrawIndexed(this->_commandBuffers[i], static_cast<uint32_t>(this->_indices.size()), 1, 0, 0, 0);
// 		/*
// 		 *	vertexCount : même si nous n'avons pas de vertex buffer, nous avons techniquement trois this->_vertices à dessiner
// instanceCount : sert au rendu instancié (instanced rendering); indiquez 1 si vous ne l'utilisez pas
// firstVertex : utilisé comme décalage dans le vertex buffer et définit ainsi la valeur la plus basse pour glVertexIndex
// firstInstance : utilisé comme décalage pour l'instanced rendering et définit ainsi la valeur la plus basse pour gl_InstanceIndex*/
// 		vkCmdEndRenderPass(this->_commandBuffers[i]);
// 		if (vkEndCommandBuffer(this->_commandBuffers[i]) != VK_SUCCESS)
// 			throw std::runtime_error("échec de l'enregistrement d'un command buffer!");
// 	}
// }

// void	Display::createFramebuffers()
// {
// 	this->_swapChainFramebuffers.resize(this->_swapChainImageViews.size());
// 	for (size_t i = 0; i < this->_swapChainImageViews.size(); i++) {
// 		std::array<VkImageView, 3> attachments = {
// 			this->_colorImageView,
//     		this->_depthImageView,
//     		this->_swapChainImageViews[i]
// 		};

// 		VkFramebufferCreateInfo framebufferInfo{};
// 		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
// 		framebufferInfo.renderPass = this->_renderPass;
// 		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
// 		framebufferInfo.pAttachments = attachments.data();
// 		framebufferInfo.width = this->_swapChainExtent.width;
// 		framebufferInfo.height = this->_swapChainExtent.height;
// 		framebufferInfo.layers = 1; //car une seule couche dans la swap chain

// 		if (vkCreateFramebuffer(this->_device.device(), &framebufferInfo, nullptr, &this->_swapChainFramebuffers[i]) != VK_SUCCESS)
// 			throw std::runtime_error("échec de la création d'un framebuffer!");
// 	}
// }

// void	Display::createRenderPass()
// {
// 	//DESCRIPTION DE L'ATTACHEMENT
// 	VkAttachmentDescription colorAttachment{};
// 	colorAttachment.format = this->_swapChainImageFormat; // qu'un attachement de couleur-> image swap chain
// 	colorAttachment.samples = this->_device.getMsaaSamples();
// 	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //action de l'attachement avant rendu (enum)
// 	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // action de l'attachement apres rendu (enum)
// 	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//stencil? on n'utilise pas
// 	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//stencil? on n'utilise pas
// 	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

// 	//Depth Attachment
// 	VkAttachmentDescription	depthAttachment{};
// 	depthAttachment.format = this->_device.findDepthFormat();
// 	depthAttachment.samples = this->_device.getMsaaSamples();
// 	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
// 	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
// 	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
// 	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
// 	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

//     VkAttachmentDescription colorAttachmentResolve{};
//     colorAttachmentResolve.format = this->_swapChainImageFormat;
//     colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
//     colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//     colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
// 	//SUBPASSES ET REFERENCES AUX ATTACHEMENTS
// 	VkAttachmentReference colorAttachmentRef{};
// 	colorAttachmentRef.attachment = 0;
// 	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

// 	VkAttachmentReference	depthAttachmentRef{};
// 	depthAttachmentRef.attachment = 1;
// 	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

// 	VkAttachmentReference colorAttachmentResolveRef{};
//     colorAttachmentResolveRef.attachment = 2;
//     colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

// 	VkSubpassDescription	subpass{};
// 	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
// 	subpass.colorAttachmentCount = 1;
// 	subpass.pColorAttachments = &colorAttachmentRef;
// 	subpass.pDepthStencilAttachment = &depthAttachmentRef;
// 	subpass.pResolveAttachments = &colorAttachmentResolveRef;

// 	//Subpass dependencies
// 	VkSubpassDependency	dependency{};
// 	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
// 	dependency.dstSubpass = 0; // dst doit etre > src sauf si VK_SUBPASS_EXTERNAL (vu que c'est avant la 0)
// 	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
// 	dependency.srcAccessMask = 0;
// 	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
// 	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

// 	//PASSE DE RENDU
// 	std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

// 	VkRenderPassCreateInfo	renderPassInfo{};
// 	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
// 	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
// 	renderPassInfo.pAttachments = attachments.data();
// 	renderPassInfo.subpassCount = 1;
// 	renderPassInfo.pSubpasses = &subpass;
// 	renderPassInfo.dependencyCount = 1;
// 	renderPassInfo.pDependencies = &dependency;

// 	if (vkCreateRenderPass(this->_device.device(), &renderPassInfo, nullptr, &this->_renderPass) != VK_SUCCESS)
// 		throw std::runtime_error("échec de la création de la render pass!");
// }

// void	Display::createGraphicsPipeline()
// {
// 	auto	vertShaderCode = readFile("shaders/vert.spv");
// 	auto	fragShaderCode = readFile("shaders/frag.spv");

// 	auto	vertShaderModule = createShaderModule(vertShaderCode); //creation du pipeline -> compile et mis sur la carte. on peut donc detruire une fois que la pipeline est finie
// 	auto	fragShaderModule = createShaderModule(fragShaderCode);

// 	VkPipelineShaderStageCreateInfo	vertShaderStageInfo{};
// 	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// 	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
// 	vertShaderStageInfo.module = vertShaderModule;
// 	vertShaderStageInfo.pName = "main";
// 	//	vertShaderStageInfo.pSpecializationInfo = nullptr; -> pour optimiser, virer du code avant la compile si pas besoin

// 	VkPipelineShaderStageCreateInfo	fragShaderStageInfo{};
// 	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
// 	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
// 	fragShaderStageInfo.module = fragShaderModule;
// 	fragShaderStageInfo.pName = "main";
// 	//fragShaderStageInfo.pSpecializationInfo = nullptr; -> pour optimiser, virer du code avant la compile si pas besoin

// 	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

// 	auto	bindingDescription = Vertex::getBindingDescription();
// 	auto	attributeDescriptions = Vertex::getAttributeDescriptions();

// 	VkPipelineVertexInputStateCreateInfo	vertexInputInfo{};
// 	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
// 	vertexInputInfo.vertexBindingDescriptionCount = 1;
// 	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
// 	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
// 	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

// 	VkPipelineInputAssemblyStateCreateInfo	inputAssembly{}; //interessant pour points / lignes / triangles / contigus ou non : https://vulkan-tutorial.com/fr/Dessiner_un_triangle/Pipeline_graphique_basique/Fonctions_fixees
// 	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
// 	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
// 	inputAssembly.primitiveRestartEnable = VK_FALSE;

// 	//VIEWPORT ET CISEAUX
// 	VkViewport viewport{}; // taille de l'affichage
// 	viewport.x = 0.0f;
// 	viewport.y = 0.0f;
// 	viewport.width = (float) this->_swapChainExtent.width;
// 	viewport.height = (float) this->_swapChainExtent.height;
// 	viewport.minDepth = 0.0f;
// 	viewport.maxDepth = 1.0f;

// 	VkRect2D scissor{};
// 	scissor.offset = {0, 0};
// 	scissor.extent = this->_swapChainExtent;

// 	VkPipelineViewportStateCreateInfo viewportState{};
// 	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
// 	viewportState.viewportCount = 1;
// 	viewportState.pViewports = &viewport;
// 	viewportState.scissorCount = 1;
// 	viewportState.pScissors = &scissor;

// 	//RASTERIZER
// 	VkPipelineRasterizationStateCreateInfo rasterizer{};
// 	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
// 	rasterizer.depthClampEnable = VK_FALSE; //shadow maps info
// 	rasterizer.rasterizerDiscardEnable = VK_FALSE; //Si le membre rasterizerDiscardEnable est mis à VK_TRUE, aucune géométrie ne passe l'étape du rasterizer, ce qui désactive purement et simplement toute émission de donnée vers le frambuffer.
// 	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // comment remplir les polygones:
// 	/* VK_POLYGON_MODE_FILL : remplit les polygones de fragments
// VK_POLYGON_MODE_LINE : les côtés des polygones sont dessinés comme des lignes
// VK_POLYGON_MODE_POINT : les sommets sont dessinées comme des points
// Tout autre mode que fill doit être activé lors de la mise en place du logical device.*/
// 	rasterizer.lineWidth = 1.0f; //taille de ligne, si autre que 1.0f, activer l'extension wideLines
// 	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
// 	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;// elements devant
// 	rasterizer.depthBiasEnable = VK_FALSE; // autre param shadow maps
// 	rasterizer.depthBiasConstantFactor = 0.0f; // Optionnel
// 	rasterizer.depthBiasClamp = 0.0f; // Optionnel
// 	rasterizer.depthBiasSlopeFactor = 0.0f; // Optionnel

// 	//MULTISAMPLING (on y repassera plus tard)
// 	VkPipelineMultisampleStateCreateInfo multisampling{};
// 	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
// 	multisampling.sampleShadingEnable = VK_TRUE;
// 	multisampling.rasterizationSamples = this->_device.getMsaaSamples();
// 	multisampling.minSampleShading = 0.2f; // Fraction minimale pour le sample shading; plus proche de 1 lisse d'autant plus
// 	multisampling.pSampleMask = nullptr; // Optionnel
// 	multisampling.alphaToCoverageEnable = VK_FALSE; // Optionnel
// 	multisampling.alphaToOneEnable = VK_FALSE; // Optionnel

// 	//COLORBLENDING (a voir pour plusieurs framebuffer
// 	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
// 	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
// 	colorBlendAttachment.blendEnable = VK_FALSE;
// 	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optionnel
// 	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optionnel
// 	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optionnel
// 	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optionnel
// 	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optionnel
// 	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optionnel

// 	VkPipelineColorBlendStateCreateInfo colorBlending{};
// 	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
// 	colorBlending.logicOpEnable = VK_FALSE;
// 	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optionnel
// 	colorBlending.attachmentCount = 1;
// 	colorBlending.pAttachments = &colorBlendAttachment;
// 	colorBlending.blendConstants[0] = 0.0f; // Optionnel
// 	colorBlending.blendConstants[1] = 0.0f; // Optionnel
// 	colorBlending.blendConstants[2] = 0.0f; // Optionnel
// 	colorBlending.blendConstants[3] = 0.0f; // Optionnel

// 	//ETATS DYNAMIQUES
// 	std::vector<VkDynamicState>	dynamicStates = {
// 		VK_DYNAMIC_STATE_VIEWPORT,
// 		VK_DYNAMIC_STATE_LINE_WIDTH
// 	};

// 	VkPipelineDynamicStateCreateInfo	dynamicState{};
// 	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
// 	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
// 	dynamicState.pDynamicStates = dynamicStates.data();

// 	//PIPELINE LAYOUT
// 	VkPipelineLayoutCreateInfo	pipelineLayoutInfo{};
// 	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
// 	pipelineLayoutInfo.setLayoutCount = 1;            // Optionnel
// 	pipelineLayoutInfo.pSetLayouts = &this->_descriptorSetLayout;
// 	pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optionnel
// 	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optionnel

// 	if (vkCreatePipelineLayout(this->_device.device(), &pipelineLayoutInfo, nullptr, &this->_pipelineLayout) != VK_SUCCESS)
// 		throw std::runtime_error("échec de la création du pipeline layout!");

// 	//gestion de profondeur
// 	VkPipelineDepthStencilStateCreateInfo	depthStencil{};
// 	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
// 	depthStencil.depthTestEnable = VK_TRUE;
// 	depthStencil.depthWriteEnable = VK_TRUE;
// 	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
// 	depthStencil.depthBoundsTestEnable = VK_FALSE;
// 	depthStencil.minDepthBounds = 0.0f; // Optionnel
// 	depthStencil.maxDepthBounds = 1.0f; // Optionnel
// 	depthStencil.stencilTestEnable = VK_FALSE;
// 	depthStencil.front = {}; // Optionnel
// 	depthStencil.back = {}; // Optionnel

// 	VkGraphicsPipelineCreateInfo pipelineInfo{};
// 	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
// 	pipelineInfo.stageCount = 2;
// 	pipelineInfo.pStages = shaderStages;

// 	pipelineInfo.pVertexInputState = &vertexInputInfo;
// 	pipelineInfo.pInputAssemblyState = &inputAssembly;
// 	pipelineInfo.pViewportState = &viewportState;
// 	pipelineInfo.pRasterizationState = &rasterizer;
// 	pipelineInfo.pMultisampleState = &multisampling;
// 	pipelineInfo.pDepthStencilState = &depthStencil;
// 	pipelineInfo.pColorBlendState = &colorBlending;
// 	//	pipelineInfo.pDynamicState = &dynamicState; // fonctionne pas atm
// 	static_cast<void>(dynamicState);
// 	pipelineInfo.pDynamicState = nullptr; // Optionnel

// 	pipelineInfo.layout = this->_pipelineLayout;

// 	pipelineInfo.renderPass = this->_renderPass; //il peut y avoir d'autres render pass mais qui sont compatible avec _renderPass voir: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap8.html#renderpass-compatibility
// 	pipelineInfo.subpass = 0;


// 	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optionnel -> voir pour utiliser une precedente pipeline pour gagner du temps, si elles se suivent
// 	pipelineInfo.basePipelineIndex = -1; // Optionnel

// 	if (vkCreateGraphicsPipelines(this->_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->_graphicsPipeline) != VK_SUCCESS)
// 		throw std::runtime_error("échec de la création de la pipeline graphique!");

	// vkDestroyShaderModule(this->_device.device(), vertShaderModule, nullptr);
	// vkDestroyShaderModule(this->_device.device(), fragShaderModule, nullptr);
// }

// VkShaderModule	Display::createShaderModule(const std::vector<char>& code) // buffer contenant le bytecode et créera un VkShaderModule avec ce code.
// {
// 	VkShaderModuleCreateInfo createInfo{};
// 	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
// 	createInfo.codeSize = code.size();
// 	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

// 	VkShaderModule	shaderModule;
// 	if (vkCreateShaderModule(this->_device.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
// 		throw std::runtime_error("échec de la création d'un module shader!");

// 	return shaderModule;
// }

// VkImageView	Display::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
// {
//     VkImageViewCreateInfo viewInfo{};
//     viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//     viewInfo.image = image;
//     viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//     viewInfo.format = format;
//     viewInfo.subresourceRange.aspectMask = aspectFlags;
//     viewInfo.subresourceRange.baseMipLevel = 0;
//     viewInfo.subresourceRange.levelCount = mipLevels;
//     viewInfo.subresourceRange.baseArrayLayer = 0;
//     viewInfo.subresourceRange.layerCount = 1;

//     VkImageView	imageView;
//     if (vkCreateImageView(this->_device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
//         throw std::runtime_error("échec de la creation de la vue sur une image!");

//     return imageView;
// }

// void	Display::createImageViews()
// {
// 	this->_swapChainImageViews.resize(this->_swapChainImages.size());

//     for (uint32_t i = 0; i < this->_swapChainImages.size(); i++) {
//         this->_swapChainImageViews[i] = this->createImageView(this->_swapChainImages[i], this->_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
//     }
// }

// void	Display::createTextureImageView()
// {
//     this->_textureImageView = this->createImageView(this->_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, this->_mipLevels);
// }

// void	Display::createSwapChain()
// {
// 	SwapChainSupportDetails	swapChainSupport = this->_device.querySwapChainSupport(this->_device.getPhysicalDevice());

// 	VkSurfaceFormatKHR	surfaceFormat = this->chooseSwapSurfaceFormat(swapChainSupport.formats);
// 	VkPresentModeKHR	presentMode = this->chooseSwapPresentMode(swapChainSupport.presentModes);
// 	VkExtent2D	extent = this->chooseSwapExtent(swapChainSupport.capabilities);

// 	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
// 	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
// 		imageCount = swapChainSupport.capabilities.maxImageCount;

// 	VkSwapchainCreateInfoKHR	createInfo{};
// 	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
// 	createInfo.surface = this->_device.getSurface();

// 	createInfo.minImageCount = imageCount;
// 	createInfo.imageFormat = surfaceFormat.format;
// 	createInfo.imageColorSpace = surfaceFormat.colorSpace;
// 	createInfo.imageExtent = extent;
// 	createInfo.imageArrayLayers = 1;
// 	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
// 	/*
// 	 * Le champ de bits imageUsage spécifie le type d'opérations que nous appliquerons aux images de la swap chain. Dans ce tutoriel nous effectuerons un rendu directement sur les images, nous les utiliserons donc comme color attachement. Vous voudrez peut-être travailler sur une image séparée pour pouvoir appliquer des effets en post-processing. Dans ce cas vous devrez utiliser une valeur comme VK_IMAGE_USAGE_TRANSFER_DST_BIT à la place et utiliser une opération de transfert de mémoire pour placer le résultat final dans une image de la swap chain.
// 	 */
// 	QueueFamilyIndices	indices = this->_device.findQueueFamilies(this->_device.getPhysicalDevice());
// 	uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

// 	if (indices.graphicsFamily != indices.presentFamily)
// 	{
// 		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // pouvoir travailler a plusieurs queues (presentation et graphique) sur la meme image (moins performant)
// 		createInfo.queueFamilyIndexCount = 2;
// 		createInfo.pQueueFamilyIndices = queueFamilyIndices;
// 	}
// 	else
// 	{
// 		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // pouvoir travailler qu'a une queue a la fois sur chaque image (plus performant et obligatoire quand 1 queue)
// 		createInfo.queueFamilyIndexCount = 0; // Optionnel
// 		createInfo.pQueueFamilyIndices = nullptr; // Optionnel
// 	}

// 	createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // si on veut transform genre rotate ou symetrie verticale
// 	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //si on veut que la fenetre influe sur les couleurs de l'image (generalement non, comme ici)
// 	createInfo.presentMode = presentMode; // meilleures performances avec clipped = vk_true
// 	createInfo.clipped = VK_TRUE; //pas afficher pixels derrieres

// 	createInfo.oldSwapchain = VK_NULL_HANDLE; // si la swap chain crash (resize par exemple), la nouvelle doit envoyer un pointer sur la precedente mais c'est complique donc on va pas le faire

// 	if (vkCreateSwapchainKHR(this->_device.device(), &createInfo, nullptr, &this->_swapChain) != VK_SUCCESS)
// 		throw std::runtime_error("failed to create swap chain!");

// 	vkGetSwapchainImagesKHR(this->_device.device(), this->_swapChain, &imageCount, nullptr);
// 	this->_swapChainImages.resize(imageCount);
// 	vkGetSwapchainImagesKHR(this->_device.device(), this->_swapChain, &imageCount, this->_swapChainImages.data());
// 	this->_swapChainImageFormat = surfaceFormat.format;
// 	this->_swapChainExtent = extent;
// }

// VkSurfaceFormatKHR	Display::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
// {
// 	for (const auto& availableFormat : availableFormats)
// 		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
// 			return availableFormat;
// 	//si ca foire, on pourrait tester d'autres formats un peux moins bien mais dans un soucis de simplicite, on prend le premier venu
// 	return availableFormats[0];
// }

// VkPresentModeKHR	Display::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
// 	for (const auto& availablePresentMode : availablePresentModes)
// 		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
// 			return availablePresentMode;
// 	//si on trouve pas le triple buffering (le meilleur atm), on pourrait en viser un autre mais son prend le mode par default, toujours present, la V-Sync
// 	return VK_PRESENT_MODE_FIFO_KHR;
// }

// VkExtent2D	Display::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
// {
// 	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
// 		return capabilities.currentExtent;
// 	else
// 	{
// 		int	width, height;
// 		glfwGetFramebufferSize(this->_window.getWindow(), &width, &height);

// 		VkExtent2D actualExtent = {
// 			static_cast<uint32_t>(width),
// 			static_cast<uint32_t>(height)
// 		};

// 		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
// 		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height); //clamp comme en css pour la taille min et max

// 		return actualExtent;
// 	}
// }