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

bool		QUIT = false;
const int	MAX_FRAMES_IN_FLIGHT = 2;

VkResult	CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static	std::vector<char> readFile(const std::string& filename) {
	std::ifstream	file(filename, std::ios::ate | std::ios::binary); //ate pour commencer a la fin / binary pour dire que c'est un binaire et pas recompiler

	if (!file.is_open())
		throw std::runtime_error(std::string {"échec de l'ouverture du fichier "} + filename + "!");
	size_t	fileSize = (size_t) file.tellg(); // on a commence a la fin donc voir ou est le pointeur
	std::vector<char>	buffer(fileSize);

	file.seekg(0); // tout lire jusqu'au debut
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

void	key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static_cast<void> (window);
	static_cast<void> (scancode);
	static_cast<void> (mods);
	if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
		QUIT = true;
}

const std::vector<Vertex>	vertices = {
	{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

Display::Display( void ) {}

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

Display::~Display() {}

void Display::run()
{
	this->initWindow();
	this->initVulkan();
	this->mainLoop();
	this->cleanup();
}

static void	framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	static_cast<void>(width);
	static_cast<void>(height);
	auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void	Display::initWindow( void )
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); //largeur, haute, titre, momiteur (si on veut un ecran particulier), propre a openGL
	glfwSetWindowUserPointer(this->_window, this);
	glfwSetFramebufferSizeCallback(this->_window, framebufferResizeCallback);
}

void	Display::initVulkan( void )
{
	this->createInstance();
	this->setupDebugMessenger();
	this->createSurface();
	this->pickPhysicalDevice();
	this->createLogicalDevice();
	this->createSwapChain();
	this->createImageViews();
	this->createRenderPass();
	this->createGraphicsPipeline();
	this->createFramebuffers();
	this->createCommandPool();
	this->createVertexBuffer();
	this->createCommandBuffers();
	this->createSyncObjects();
}

void	Display::mainLoop( void )
{
	while (!glfwWindowShouldClose(this->_window) && !QUIT) {
		glfwSetKeyCallback(this->_window, key_callback);
		glfwPollEvents();
		this->drawFrame();
	}

	vkDeviceWaitIdle(this->_device); //attente de la fin des semaphores pour quitter
}

void	Display::cleanup( void )
{
	this->cleanupSwapChain();

	vkDestroyBuffer(this->_device, this->_vertexBuffer, nullptr);
	vkFreeMemory(this->_device, this->_vertexBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(this->_device, this->_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(this->_device, this->_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(this->_device, this->_inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(this->_device, this->_commandPool, nullptr);

	vkDestroyDevice(this->_device, nullptr);

	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(this->_instance, this->_debugMessenger, nullptr);

	vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
	vkDestroyInstance(this->_instance, nullptr);

	glfwDestroyWindow(this->_window);

	glfwTerminate();
}

void Display::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("validation layers requested, but not available!");

	VkApplicationInfo	appInfo{}; // informations optionnelles mais utiles pour optimiser
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // expliciter le type
	appInfo.pApplicationName = "Hello Triangle"; // nom de l'app
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // version
	appInfo.pEngineName = "No Engine"; // engine si utilise
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo	createInfo{}; // structure permettant la création de l'instance. Celle-ci n'est pas optionnelle. -> informer le driver des extensions et des validation layers
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT	debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	} else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &this->_instance) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");
	//Pointeur sur une structure contenant l'information pour la création
	//Pointeur sur une fonction d'allocation que nous laisserons toujours nullptr
	//Pointeur sur une variable stockant une référence au nouvel objet
}

void	Display::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(this->_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("echec de la creation d'un buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(this->_device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(this->_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("echec de l'allocation de memoire!");
	}

	vkBindBufferMemory(this->_device, buffer, bufferMemory, 0);
	/*
	 * Le quatrième indique le décalage entre le début de la mémoire et le début du buffer. Nous avons alloué cette mémoire spécialement pour ce buffer, nous pouvons donc mettre 0. Si vous décidez d'allouer un grand espace mémoire pour y mettre plusieurs buffers, sachez qu'il faut que ce nombre soit divisible par memRequirements.alignement. Notez que cette stratégie est la manière recommandée de gérer la mémoire des GPUs (https://developer.nvidia.com/vulkan-memory-management)
	 */
}

uint32_t	Display::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties	memProperties;
	//on recupere les types de memoire de la CG
	//on ne va pas le faire mais on peut choisir celle qui est la plus performante !
	vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memProperties);

	//on cherche un type de memoire qui correspond au buffer
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
}

void 	Display::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo	allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(this->_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optionnel
	copyRegion.dstOffset = 0; // Optionnel
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo	submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->_graphicsQueue);
	vkFreeCommandBuffers(this->_device, this->_commandPool, 1, &commandBuffer);
}

void	Display::createVertexBuffer()
{
	VkDeviceSize	bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer	stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	/*
	 * Cette fonction nous permet d'accéder à une région spécifique d'une ressource. Nous devons pour cela indiquer un décalage et une taille. Nous mettons ici respectivement 0 et bufferInfo.size. Il est également possible de fournir la valeur VK_WHOLE_SIZE pour mapper d'un coup toute la ressource. L'avant-dernier paramètre est un champ de bits pour l'instant non implémenté par Vulkan. Il est impératif de la laisser à 0. Enfin, le dernier paramètre permet de fournir un pointeur vers la mémoire ainsi mappée.
	 */
	vkMapMemory(this->_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(_device, stagingBufferMemory);
	/*
	 * Vous pouvez maintenant utiliser memcpy pour copier les vertices dans la mémoire, puis démapper le buffer à l'aide de vkUnmapMemory. Malheureusement le driver peut décider de cacher les données avant de les copier dans le buffer. Il est aussi possible que les données soient copiées mais que ce changement ne soit pas visible immédiatement. Il y a deux manières de régler ce problème :

	 Utiliser une pile de mémoire cohérente avec la RAM, ce qui est indiqué par VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	 Appeler vkFlushMappedMemoryRanges après avoir copié les données, puis appeler vkInvalidateMappedMemory avant d'accéder à la mémoire
	 */
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_vertexBuffer, this->_vertexBufferMemory);

	copyBuffer(stagingBuffer, this->_vertexBuffer, bufferSize);
	vkDestroyBuffer(this->_device, stagingBuffer, nullptr);
	vkFreeMemory(this->_device, stagingBufferMemory, nullptr);
}

void	Display::recreateSwapChain()
{
	// Quand fenetre minimisee, mise en pause du rendu
	int	width = 0;
	int	height = 0;
	glfwGetFramebufferSize(this->_window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(this->_window, &width, &height);
		glfwWaitEvents();
	}
	//

	vkDeviceWaitIdle(this->_device);

	this->cleanupSwapChain();

	this->createSwapChain();
	this->createImageViews();
	this->createRenderPass();
	this->createGraphicsPipeline();
	this->createFramebuffers();
	this->createCommandBuffers();
}

void	Display::cleanupSwapChain()
{
	for (size_t i = 0; i < this->_swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(this->_device, this->_swapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(this->_device, this->_commandPool, static_cast<uint32_t>(this->_commandBuffers.size()), this->_commandBuffers.data());

	vkDestroyPipeline(this->_device, this->_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(this->_device, this->_pipelineLayout, nullptr);
	vkDestroyRenderPass(this->_device, this->_renderPass, nullptr);

	for (size_t i = 0; i < this->_swapChainImageViews.size(); i++)
		vkDestroyImageView(this->_device, this->_swapChainImageViews[i], nullptr);

	vkDestroySwapchainKHR(this->_device, this->_swapChain, nullptr);
}

void	Display::createSyncObjects()
{
	this->_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	this->_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	this->_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	this->_imagesInFlight.resize(this->_swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(this->_device, &semaphoreInfo, nullptr, &this->_imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(this->_device, &semaphoreInfo, nullptr, &this->_renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(this->_device, &fenceInfo, nullptr, &this->_inFlightFences[i]) != VK_SUCCESS)

			throw std::runtime_error("échec de la création des objets de synchronisation pour une frame!");
	}
}

void	Display::drawFrame()
{
	/*
	   1)Acquérir une image depuis la swap chain
	   2)Exécuter le command buffer correspondant au framebuffer dont l'attachement est l'image obtenue
	   3)Retourner l'image à la swap chain pour présentation
	   */
	vkWaitForFences(this->_device, 1, &this->_inFlightFences[this->_currentFrame], VK_TRUE, UINT64_MAX);//VK_TRUE permet d'attendre que TOUTES les fences soient good

	uint32_t	imageIndex;
	VkResult	result = vkAcquireNextImageKHR(this->_device, this->_swapChain, UINT64_MAX, this->_imageAvailableSemaphores[this->_currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("échec de la présentation d'une image à la swap chain!");
	}

	// Vérifier si une frame précédente est en train d'utiliser cette image (il y a une fence à attendre)
	if (this->_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(this->_device, 1, &this->_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	// Marque l'image comme étant à nouveau utilisée par cette frame
	this->_imagesInFlight[imageIndex] = this->_inFlightFences[this->_currentFrame];

	//Envoi du command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {this->_imageAvailableSemaphores[this->_currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->_commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = {this->_renderFinishedSemaphores[this->_currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(this->_device, 1, &this->_inFlightFences[this->_currentFrame]);//obliger de rester a la main
	if (vkQueueSubmit(this->_graphicsQueue, 1, &submitInfo, this->_inFlightFences[this->_currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("échec de l'envoi d'un command buffer!");

	//Presentation
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {this->_swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optionnel

	//	VkResult	result = vkAcquireNextImageKHR(this->_device, this->_swapChain, UINT64_MAX, this->_imageAvailableSemaphores[this->_currentFrame], VK_NULL_HANDLE, &imageIndex);

	result = vkQueuePresentKHR(this->_presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->framebufferResized)
	{
		this->framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("échec de la présentation d'une image à la swap chain!");
	this->_currentFrame = (this->_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void	Display::createCommandBuffers()
{
	this->_commandBuffers.resize(this->_swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo	allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//VK_COMMAND_BUFFER_LEVEL_PRIMARY : peut être envoyé à une queue pour y être exécuté mais ne peut être appelé par d'autres command buffers
	//VK_COMMAND_BUFFER_LEVEL_SECONDARY : ne peut pas être directement émis à une queue mais peut être appelé par un autre command buffer
	allocInfo.commandBufferCount = (uint32_t) this->_commandBuffers.size();

	if (vkAllocateCommandBuffers(this->_device, &allocInfo, this->_commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("échec de l'allocation de command buffers!");

	//Début de l'enregistrement des commandes

	for (size_t i = 0; i < this->_commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo	beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optionnel
		beginInfo.pInheritanceInfo = nullptr; // Optionel

		if (vkBeginCommandBuffer(this->_commandBuffers[i], &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("erreur au début de l'enregistrement d'un command buffer!");
		//Commencer une render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = this->_renderPass;
		renderPassInfo.framebuffer = this->_swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = this->_swapChainExtent;
		VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		vkCmdBeginRenderPass(this->_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		//VK_SUBPASS_CONTENTS_INLINE : les commandes de la render pass seront inclues directement dans le command buffer (qui est donc primaire)
		//VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFER : les commandes de la render pass seront fournies par un ou plusieurs command buffers secondaires

		//Commandes d'affichage basiques
		vkCmdBindPipeline(this->_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->_graphicsPipeline);
		VkBuffer	vertexBuffers[] = {this->_vertexBuffer};
		VkDeviceSize	offsets[] = {0};
		vkCmdBindVertexBuffers(this->_commandBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdDraw(this->_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		/*
		 *	vertexCount : même si nous n'avons pas de vertex buffer, nous avons techniquement trois vertices à dessiner
instanceCount : sert au rendu instancié (instanced rendering); indiquez 1 si vous ne l'utilisez pas
firstVertex : utilisé comme décalage dans le vertex buffer et définit ainsi la valeur la plus basse pour glVertexIndex
firstInstance : utilisé comme décalage pour l'instanced rendering et définit ainsi la valeur la plus basse pour gl_InstanceIndex*/
		vkCmdEndRenderPass(this->_commandBuffers[i]);
		if (vkEndCommandBuffer(this->_commandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("échec de l'enregistrement d'un command buffer!");
	}
}

void	Display::createCommandPool()
{
	QueueFamilyIndices	queueFamilyIndices = findQueueFamilies(this->_physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : informe que les command buffers sont ré-enregistrés très souvent, ce qui peut inciter Vulkan (et donc le driver) à ne pas utiliser le même type d'allocation
	//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : permet aux command buffers d'être ré-enregistrés individuellement, ce que les autres configurations ne permettent pas
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optionel

	if (vkCreateCommandPool(this->_device, &poolInfo, nullptr, &this->_commandPool) != VK_SUCCESS)
		throw std::runtime_error("échec de la création d'une command pool!");
}

void	Display::createFramebuffers()
{
	this->_swapChainFramebuffers.resize(this->_swapChainImageViews.size());
	for (size_t i = 0; i < this->_swapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			this->_swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = this->_swapChainExtent.width;
		framebufferInfo.height = this->_swapChainExtent.height;
		framebufferInfo.layers = 1; //car une seule couche dans la swap chain

		if (vkCreateFramebuffer(this->_device, &framebufferInfo, nullptr, &this->_swapChainFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("échec de la création d'un framebuffer!");
	}

}

void	Display::createRenderPass()
{
	//DESCRIPTION DE L'ATTACHEMENT
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = this->_swapChainImageFormat; // qu'un attachement de couleur-> image swap chain
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //pour l'instant un seul sample
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //action de l'attachement avant rendu (enum)
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // action de l'attachement apres rendu (enum)
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//stencil? on n'utilise pas
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//stencil? on n'utilise pas
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//SUBPASSES ET REFERENCES AUX ATTACHEMENTS
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; //parce qu'un seul attachement
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	//PASSE DE RENDU
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	//Subpass dependencies
	VkSubpassDependency	dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0; // dst doit etre > src sauf si VK_SUBPASS_EXTERNAL (vu que c'est avant la 0)
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(this->_device, &renderPassInfo, nullptr, &this->_renderPass) != VK_SUCCESS)
		throw std::runtime_error("échec de la création de la render pass!");
}

void	Display::createGraphicsPipeline()
{
	auto	vertShaderCode = readFile("shaders/vert.spv");
	auto	fragShaderCode = readFile("shaders/frag.spv");

	auto	vertShaderModule = createShaderModule(vertShaderCode); //creation du pipeline -> compile et mis sur la carte. on peut donc detruire une fois que la pipeline est finie
	auto	fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo	vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	//	vertShaderStageInfo.pSpecializationInfo = nullptr; -> pour optimiser, virer du code avant la compile si pas besoin

	VkPipelineShaderStageCreateInfo	fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	//fragShaderStageInfo.pSpecializationInfo = nullptr; -> pour optimiser, virer du code avant la compile si pas besoin

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	auto	bindingDescription = Vertex::getBindingDescription();
	auto	attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo	vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo	inputAssembly{}; //interessant pour points / lignes / triangles / contigus ou non : https://vulkan-tutorial.com/fr/Dessiner_un_triangle/Pipeline_graphique_basique/Fonctions_fixees
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//VIEWPORT ET CISEAUX
	VkViewport viewport{}; // taille de l'affichage
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) this->_swapChainExtent.width;
	viewport.height = (float) this->_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = this->_swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//RASTERIZER
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; //shadow maps info
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //Si le membre rasterizerDiscardEnable est mis à VK_TRUE, aucune géométrie ne passe l'étape du rasterizer, ce qui désactive purement et simplement toute émission de donnée vers le frambuffer.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // comment remplir les polygones:
	/* VK_POLYGON_MODE_FILL : remplit les polygones de fragments
VK_POLYGON_MODE_LINE : les côtés des polygones sont dessinés comme des lignes
VK_POLYGON_MODE_POINT : les sommets sont dessinées comme des points
Tout autre mode que fill doit être activé lors de la mise en place du logical device.*/
	rasterizer.lineWidth = 1.0f; //taille de ligne, si autre que 1.0f, activer l'extension wideLines
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;// elements devant
	rasterizer.depthBiasEnable = VK_FALSE; // autre param shadow maps
	rasterizer.depthBiasConstantFactor = 0.0f; // Optionnel
	rasterizer.depthBiasClamp = 0.0f; // Optionnel
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optionnel

	//MULTISAMPLING (on y repassera plus tard)
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optionnel
	multisampling.pSampleMask = nullptr; // Optionnel
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optionnel
	multisampling.alphaToOneEnable = VK_FALSE; // Optionnel

	//COLORBLENDING (a voir pour plusieurs framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optionnel
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optionnel
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optionnel
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optionnel
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optionnel
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optionnel

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optionnel
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optionnel
	colorBlending.blendConstants[1] = 0.0f; // Optionnel
	colorBlending.blendConstants[2] = 0.0f; // Optionnel
	colorBlending.blendConstants[3] = 0.0f; // Optionnel

	//ETATS DYNAMIQUES
	std::vector<VkDynamicState>	dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo	dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	//PIPELINE LAYOUT
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;            // Optionnel
	pipelineLayoutInfo.pSetLayouts = nullptr;         // Optionnel
	pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optionnel
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optionnel

	if (vkCreatePipelineLayout(this->_device, &pipelineLayoutInfo, nullptr, &this->_pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("échec de la création du pipeline layout!");

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optionnel
	pipelineInfo.pColorBlendState = &colorBlending;
	//	pipelineInfo.pDynamicState = &dynamicState; // fonctionne pas atm
	static_cast<void>(dynamicState);
	pipelineInfo.pDynamicState = nullptr; // Optionnel

	pipelineInfo.layout = this->_pipelineLayout;

	pipelineInfo.renderPass = this->_renderPass; //il peut y avoir d'autres render pass mais qui sont compatible avec _renderPass voir: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap8.html#renderpass-compatibility
	pipelineInfo.subpass = 0;


	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optionnel -> voir pour utiliser une precedente pipeline pour gagner du temps, si elles se suivent
	pipelineInfo.basePipelineIndex = -1; // Optionnel

	if (vkCreateGraphicsPipelines(this->_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->_graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("échec de la création de la pipeline graphique!");

	vkDestroyShaderModule(this->_device, vertShaderModule, nullptr);
	vkDestroyShaderModule(this->_device, fragShaderModule, nullptr);
}

VkShaderModule	Display::createShaderModule(const std::vector<char>& code) // buffer contenant le bytecode et créera un VkShaderModule avec ce code.
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule	shaderModule;
	if (vkCreateShaderModule(this->_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("échec de la création d'un module shader!");

	return shaderModule;
}

void	Display::createImageViews()
{
	this->_swapChainImageViews.resize(this->_swapChainImages.size());

	for (size_t i = 0; i < this->_swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = this->_swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; //ici 3D ?
		createInfo.format = this->_swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		/*
		 * Si vous travailliez sur une application 3D stéréoscopique, vous devrez alors créer une swap chain avec plusieurs couches. Vous pourriez alors créer plusieurs image views pour chaque image. Elles représenteront ce qui sera affiché pour l'œil gauche et pour l'œil droit.
		 */
		if (vkCreateImageView(this->_device, &createInfo, nullptr, &this->_swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("échec de la création d'une image view!");
	}

}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( // les prototypes permettent de compiler sur tous les os
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
{
	static_cast<void>(messageSeverity);
	static_cast<void>(messageType);
	static_cast<void>(pUserData);
	/*
	   Le premier paramètre indique la sévérité du message, et peut prendre les valeurs suivantes :

VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Message de suivi des appels
VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Message d'information (allocation d'une ressource...)
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message relevant un comportement qui n'est pas un bug mais plutôt une imperfection involontaire
VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message relevant un comportement invalide pouvant mener à un crash
Les valeurs de cette énumération on été conçues de telle sorte qu'il est possible de les comparer pour vérifier la sévérité d'un message, par exemple :

if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
	// Le message est suffisamment important pour être affiché
	}
	*/
	/*
	   Le paramètre messageType peut prendre les valeurs suivantes :

VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT : Un événement quelconque est survenu, sans lien avec les performances ou la spécification
VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : Une violation de la spécification ou une potentielle erreur est survenue
VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : Utilisation potentiellement non optimale de Vulka
*/
	/*
	   Le paramètre pCallbackData est une structure du type VkDebugUtilsMessengerCallbackDataEXT contenant les détails du message. Ses membres les plus importants sont :

pMessage: Le message sous la forme d'une chaîne de type C terminée par le caractère nul \0
pObjects: Un tableau d'objets Vulkan liés au message
objectCount: Le nombre d'objets dans le tableau précédent
*/
	/*
	   Finalement, le paramètre pUserData est un pointeur sur une donnée quelconque que vous pouvez spécifier à la création de la fonction de rappel.
	   */
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	// l'idee est de savoir si l'erreur nous oblige a stopper la fenetre, elle teste surtout les layers donc is ok
	return VK_FALSE;
}

void	Display::createSwapChain()
{
	SwapChainSupportDetails	swapChainSupport = this->querySwapChainSupport(this->_physicalDevice);

	VkSurfaceFormatKHR	surfaceFormat = this->chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR	presentMode = this->chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D	extent = this->chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = this->_surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	/*
	 * Le champ de bits imageUsage spécifie le type d'opérations que nous appliquerons aux images de la swap chain. Dans ce tutoriel nous effectuerons un rendu directement sur les images, nous les utiliserons donc comme color attachement. Vous voudrez peut-être travailler sur une image séparée pour pouvoir appliquer des effets en post-processing. Dans ce cas vous devrez utiliser une valeur comme VK_IMAGE_USAGE_TRANSFER_DST_BIT à la place et utiliser une opération de transfert de mémoire pour placer le résultat final dans une image de la swap chain.
	 */
	QueueFamilyIndices	indices = findQueueFamilies(this->_physicalDevice);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // pouvoir travailler a plusieurs queues (presentation et graphique) sur la meme image (moins performant)
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // pouvoir travailler qu'a une queue a la fois sur chaque image (plus performant et obligatoire quand 1 queue)
		createInfo.queueFamilyIndexCount = 0; // Optionnel
		createInfo.pQueueFamilyIndices = nullptr; // Optionnel
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // si on veut transform genre rotate ou symetrie verticale
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //si on veut que la fenetre influe sur les couleurs de l'image (generalement non, comme ici)
	createInfo.presentMode = presentMode; // meilleures performances avec clipped = vk_true
	createInfo.clipped = VK_TRUE; //pas afficher pixels derrieres

	createInfo.oldSwapchain = VK_NULL_HANDLE; // si la swap chain crash (resize par exemple), la nouvelle doit envoyer un pointer sur la precedente mais c'est complique donc on va pas le faire

	if (vkCreateSwapchainKHR(this->_device, &createInfo, nullptr, &this->_swapChain) != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain!");

	vkGetSwapchainImagesKHR(this->_device, this->_swapChain, &imageCount, nullptr);
	this->_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(this->_device, this->_swapChain, &imageCount, this->_swapChainImages.data());
	this->_swapChainImageFormat = surfaceFormat.format;
	this->_swapChainExtent = extent;
}

void	Display::createSurface()
{
	if (glfwCreateWindowSurface(this->_instance, this->_window, nullptr, &this->_surface) != VK_SUCCESS)
		throw std::runtime_error("échec de la création de la window surface!");
}

void	Display::createLogicalDevice()
{
	QueueFamilyIndices	indices = findQueueFamilies(this->_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo>	queueCreateInfos;
	std::set<uint32_t>	uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}
	if (vkCreateDevice(this->_physicalDevice, &createInfo, nullptr, &this->_device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	vkGetDeviceQueue(this->_device, indices.graphicsFamily.value(), 0, &this->_graphicsQueue); //0 est l'index, qu'une queue ici donc juste 0
	vkGetDeviceQueue(this->_device, indices.presentFamily.value(), 0, &this->_presentQueue);
}

void	Display::pickPhysicalDevice( void )
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, nullptr);
	if (deviceCount == 0) //si pas de carte graphique rien ne sert de lancer
		throw std::runtime_error("aucune carte graphique ne supporte Vulkan!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, devices.data());

	//selection des cg

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			this->_physicalDevice = device;
			break;
		}
	}

	// L'utilisation d'une map permet de les trier automatiquement de manière ascendante
	std::multimap<int, VkPhysicalDevice>	candidates;

	for (const auto& device : devices) {
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Voyons si la meilleure possède les fonctionnalités dont nous ne pouvons nous passer
	if (candidates.rbegin()->first > 0) {
		this->_physicalDevice = candidates.rbegin()->second;
	} else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

int	Display::rateDeviceSuitability(VkPhysicalDevice device)
{
	int score = 0;
	VkPhysicalDeviceFeatures	deviceFeatures;
	VkPhysicalDeviceProperties	deviceProperties;

	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Les carte graphiques dédiées ont un énorme avantage en terme de performances
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// La taille maximale des textures affecte leur qualité
	score += deviceProperties.limits.maxImageDimension2D;

	// L'application (fictive) ne peut fonctionner sans les geometry shaders
	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	return score;
}
//contraintes que devront remplir les physical devices.
bool	Display::isDeviceSuitable(VkPhysicalDevice device)
{
	bool	extensionsSupported = checkDeviceExtensionSupport(device);

	VkPhysicalDeviceProperties	deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures	deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	bool	swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

VkSurfaceFormatKHR	Display::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	//si ca foire, on pourrait tester d'autres formats un peux moins bien mais dans un soucis de simplicite, on prend le premier venu
	return availableFormats[0];
}

VkPresentModeKHR	Display::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes)
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	//si on trouve pas le triple buffering (le meilleur atm), on pourrait en viser un autre mais son prend le mode par default, toujours present, la V-Sync
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D	Display::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		int	width, height;
		glfwGetFramebufferSize(this->_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height); //clamp comme en css pour la taille min et max

		return actualExtent;
	}
}

bool	Display::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t	extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails	Display::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails	details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->_surface, &details.capabilities);

	uint32_t	formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->_surface, &formatCount, details.formats.data());
	}

	uint32_t	presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

QueueFamilyIndices	Display::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices	indices;

	uint32_t	queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int	i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->_surface, &presentSupport);
		if (presentSupport)
			indices.presentFamily = i;
		if (indices.isComplete())
			break;
		i++;
	}

	return indices;
}

void Display::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void Display::setupDebugMessenger() {
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(this->_instance, &createInfo, nullptr, &this->_debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("failed to set up debug messenger!");
}

std::vector<const char*>	Display::getRequiredExtensions()
{
	uint32_t	glfwExtensionCount = 0;
	const char**	glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

bool	Display::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;
}
