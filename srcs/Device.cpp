/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Device.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 15:22:12 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 15:22:12 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Device.hpp"

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback( // les prototypes permettent de compiler sur tous les os
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
	#ifdef DEBUG
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	#endif
	// l'idee est de savoir si l'erreur nous oblige a stopper la fenetre, elle teste surtout les layers donc is ok
	return VK_FALSE;
}

VkResult	CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pCallback)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void	DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

// class member functions
ft_Device::ft_Device(ft_Window &window): _window{window}
{
    char	status = 0;
    createInstance();
    try {
        setupDebugMessenger();
        ++status;
        createSurface();
        ++status;
        pickPhysicalDevice();
        createLogicalDevice();
        ++status;
        createCommandPool();
    }
    catch (const std::exception& e) {
        if (status > 2)
            vkDestroyDevice(this->_device_, nullptr);
        if (enableValidationLayers && status)
            DestroyDebugUtilsMessengerEXT(this->_instance, this->_debugMessenger, nullptr);
        if (status > 1)
            vkDestroySurfaceKHR(this->_instance, this->_surface_, nullptr);
        vkDestroyInstance(this->_instance, nullptr);
        throw std::runtime_error(e.what());
    }
}

ft_Device::~ft_Device()
{
	vkDestroyCommandPool(this->_device_, this->_commandPool, nullptr);
	vkDestroyDevice(this->_device_, nullptr);

	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(this->_instance, this->_debugMessenger, nullptr);

	vkDestroySurfaceKHR(this->_instance, this->_surface_, nullptr);
	vkDestroyInstance(this->_instance, nullptr);
}

void	ft_Device::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("validation layers requested, but not available!");

	VkApplicationInfo	appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,// Optimization informations (optional). stype = specify the type
								.pApplicationName = "SCOP",// name of the app
								.applicationVersion = VK_MAKE_VERSION(1, 0, 0),// version
								.pEngineName = "No Engine",// engine if in use
								.engineVersion = VK_MAKE_VERSION(1, 0, 0),
								.apiVersion = VK_API_VERSION_1_0}; 

	std::vector<const char *>	extensions = getRequiredExtensions();

	VkInstanceCreateInfo	createInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,// Struct to create instance. Mandator. -> to inform the driver the extensions and validation layers
										.pNext = nullptr,
										.pApplicationInfo = &appInfo,
										.enabledLayerCount = 0,
										.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
										.ppEnabledExtensionNames = extensions.data()};

	VkDebugUtilsMessengerCreateInfoEXT	debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(this->_validationLayers.size());
		createInfo.ppEnabledLayerNames = this->_validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}

	if (vkCreateInstance(&createInfo, nullptr, &this->_instance) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");

  	hasGflwRequiredInstanceExtensions();
}

void	ft_Device::pickPhysicalDevice()
{
	uint32_t	deviceCount = 0;
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, nullptr);
	if (deviceCount == 0) //si pas de carte graphique rien ne sert de lancer
    	throw std::runtime_error("failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice>	devices(deviceCount);
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, devices.data());

	//Select Graphic Card

	for (const VkPhysicalDevice& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			this->_physicalDevice = device;
			this->_msaaSamples = getMaxUsableSampleCount(VK_SAMPLE_COUNT_8_BIT);
			break;
		}
	}

	// L'utilisation d'une map permet de les trier automatiquement de manière ascendante
	std::multimap<int, VkPhysicalDevice>	candidates;

	for (const VkPhysicalDevice& device : devices)
	{
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Voyons si la meilleure possède les fonctionnalités dont nous ne pouvons nous passer
	if (candidates.rbegin()->first > 0)
		this->_physicalDevice = candidates.rbegin()->second;
	else
		throw std::runtime_error("failed to find a suitable GPU!");
}

VkSampleCountFlagBits	ft_Device::getMaxUsableSampleCount(VkSampleCountFlags requestedSampleCount)
{
    VkPhysicalDeviceProperties	physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(this->_physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	counts /= 2;

    if (requestedSampleCount & counts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (requestedSampleCount & counts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (requestedSampleCount & counts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (requestedSampleCount & counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;
    if (requestedSampleCount & counts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    if (requestedSampleCount & counts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    // If the requested sample count is not directly supported, find the nearest lower supported count
    if (counts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

int	ft_Device::rateDeviceSuitability(VkPhysicalDevice device)
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

void	ft_Device::createLogicalDevice()
{
	QueueFamilyIndices	indices = findQueueFamilies(this->_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo>	queueCreateInfos;
	std::set<uint64_t>	uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

	float queuePriority = 1.f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
												.queueFamilyIndex = queueFamily,
												.queueCount = 1,
												.pQueuePriorities = &queuePriority};
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{.sampleRateShading = VK_TRUE,
											.samplerAnisotropy = VK_TRUE};

	VkDeviceCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
									.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
									.pQueueCreateInfos = queueCreateInfos.data(),
									.enabledLayerCount = 0,
									.enabledExtensionCount = static_cast<uint32_t>(this->_deviceExtensions.size()),
									.ppEnabledExtensionNames = this->_deviceExtensions.data(),
									.pEnabledFeatures = &deviceFeatures};

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(this->_validationLayers.size());
		createInfo.ppEnabledLayerNames = this->_validationLayers.data();
	}
	if (vkCreateDevice(this->_physicalDevice, &createInfo, nullptr, &this->_device_) != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");
	vkGetDeviceQueue(this->_device_, indices.graphicsFamily, 0, &this->_graphicsQueue_);
	vkGetDeviceQueue(this->_device_, indices.presentFamily, 0, &this->_presentQueue_);
}

void	ft_Device::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    VkCommandPoolCreateInfo	poolInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
										.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
										.queueFamilyIndex = queueFamilyIndices.graphicsFamily};
    //VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : informe que les command buffers sont ré-enregistrés très souvent, ce qui peut inciter Vulkan (et donc le driver) à ne pas utiliser le même type d'allocation
    //VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : permet aux command buffers d'être ré-enregistrés individuellement, ce que les autres configurations ne permettent pas
    if (vkCreateCommandPool(this->_device_, &poolInfo, nullptr, &this->_commandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

void	ft_Device::createSurface()
{
    this->_window.createWindowSurface(this->_instance, &this->_surface_);
}

bool	ft_Device::isDeviceSuitable(VkPhysicalDevice device)
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

	return indices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
}

void    ft_Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
					.pfnUserCallback = debugCallback,
					.pUserData = nullptr}; //optional
}

void    ft_Device::setupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(this->_instance, &createInfo, nullptr, &this->_debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("failed to set up DEBUG messenger!");
}

bool    ft_Device::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : this->_validationLayers)
	{
		bool layerFound = false;

		for (const VkLayerProperties& layerProperties : availableLayers)
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

std::vector<const char *>   ft_Device::getRequiredExtensions()
{
	uint32_t	    glfwExtensionCount = 0;
	const char**	glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*>	extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void    ft_Device::hasGflwRequiredInstanceExtensions()
{
	uint32_t  extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties>	extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    #ifdef DEBUG
	  std::cout << "available extensions:" << std::endl;
    #endif
	std::unordered_set<std::string>	available;
	for (const VkExtensionProperties &extension : extensions) {
		#ifdef DEBUG
			std::cout << "\t" << extension.extensionName << std::endl;
    	#endif
		available.insert(extension.extensionName);
	}
    #ifdef DEBUG
		std::cout << "required extensions:" << std::endl;
    #endif
	std::vector<const char *> requiredExtensions = getRequiredExtensions();
	for (const auto &required : requiredExtensions)
	{
		#ifdef DEBUG
			std::cout << "\t" << required << std::endl;
    	#endif
		if (available.find(required) == available.end())
			throw std::runtime_error("Missing required glfw extension");
	}
}

bool    ft_Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t	extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties>    availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		device,
		nullptr,
		&extensionCount,
		availableExtensions.data());

	std::set<std::string>	requiredExtensions(this->_deviceExtensions.begin(), this->_deviceExtensions.end());

	for (const VkExtensionProperties &extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

QueueFamilyIndices	ft_Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices	indices;
    uint32_t			queueFamilyCount = 0;
    
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties>	queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t	i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->_surface_, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }
        if (indices.isComplete()) {
        	break;
        }
        i++;
    }

    return indices;
}

SwapChainSupportDetails	ft_Device::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails	details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->_surface_, &details.capabilities);

	uint32_t	formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->_surface_, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->_surface_, &formatCount, details.formats.data());
	}

	uint32_t	presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->_surface_, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->_surface_, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkFormat	ft_Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
    	VkFormatProperties	props;
    	vkGetPhysicalDeviceFormatProperties(this->_physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}
    throw std::runtime_error("failed to find supported format!");
}

uint32_t    ft_Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties	memProperties;
	//on recupere les types de memoire de la CG
	vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memProperties);

	//on cherche un type de memoire qui correspond au buffer
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
  	throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat	ft_Device::findDepthFormat()
{
	return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void    ft_Device::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory)
{
	VkBufferCreateInfo	bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
									.size = size,
									.usage = usage,
									.sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	if (vkCreateBuffer(this->_device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer!");

	VkMemoryRequirements	memRequirements;
	vkGetBufferMemoryRequirements(this->_device_, buffer, &memRequirements);

	try {
		VkMemoryAllocateInfo	allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
											.allocationSize = memRequirements.size,
											.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};

		if (vkAllocateMemory(this->_device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate vertex buffer memory!");

		vkBindBufferMemory(this->_device_, buffer, bufferMemory, 0);
	}
	catch (const std::exception& e) {
   		vkDestroyBuffer(this->_device_, buffer, nullptr);
    	vkFreeMemory(this->_device_, bufferMemory, nullptr);
		throw std::runtime_error(e.what());
	}
	/*
	 * Le quatrième indique le décalage entre le début de la mémoire et le début du buffer.
	 Nous avons alloué cette mémoire spécialement pour ce buffer, nous pouvons donc mettre 0.
	 Si vous décidez d'allouer un grand espace mémoire pour y mettre plusieurs buffers, sachez qu'il faut que ce nombre soit divisible par memRequirements.alignement.
	 Notez que cette stratégie est la manière recommandée de gérer la mémoire des GPUs (https://developer.nvidia.com/vulkan-memory-management)
	 */
}

VkCommandBuffer	ft_Device::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo	allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
											.commandPool = this->_commandPool,
											.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
											.commandBufferCount = 1};

    VkCommandBuffer	commandBuffer;
    vkAllocateCommandBuffers(this->_device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo	beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
											.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return	commandBuffer;
}

void	ft_Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo	submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
								.commandBufferCount = 1,
								.pCommandBuffers = &commandBuffer};

    vkQueueSubmit(this->_graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->_graphicsQueue_);

    vkFreeCommandBuffers(this->_device_, this->_commandPool, 1, &commandBuffer);
}

void    ft_Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer	commandBuffer = beginSingleTimeCommands();

    VkBufferCopy	copyRegion{.srcOffset = 0,
								.dstOffset = 0,
								.size = size};
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void    ft_Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{.bufferOffset = 0,
								.bufferRowLength = 0,
								.bufferImageHeight = 0,
								.imageSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
													.mipLevel = 0,
													.baseArrayLayer = 0,
													.layerCount = layerCount},
								.imageOffset = {0, 0, 0},
								.imageExtent = {width, height, 1}};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    endSingleTimeCommands(commandBuffer);
}

void    ft_Device::createImageWithInfo(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory)
{
    if (vkCreateImage(this->device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");

    VkMemoryRequirements	memRequirements;
    vkGetImageMemoryRequirements(this->_device_, image, &memRequirements);

    VkMemoryAllocateInfo	allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
										.allocationSize = memRequirements.size,
										.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};

    if (vkAllocateMemory(this->device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");

    if (vkBindImageMemory(this->_device_, image, imageMemory, 0) != VK_SUCCESS)
        throw std::runtime_error("failed to bind image memory!");
}