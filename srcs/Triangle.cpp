/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Triangle.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:10:26 by nflan             #+#    #+#             */
/*   Updated: 2023/10/25 16:13:43 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Triangle.hpp"

VkResult CreateDebugUtilsMessengerEXT(
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

bool	QUIT = false;

void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

void	key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
		QUIT = true;
};

Triangle::Triangle( void ) {
};

Triangle::Triangle( const Triangle & o) {
	if (this != &o)
		*this = o;
	return ;
}

Triangle::~Triangle() {};

void Triangle::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void	Triangle::initWindow( void ) {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); //largeur, haute, titre, momiteur (si on veut un ecran particulier), propre a openGL

};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( // les prototypes permettent de compiler sur tous les os
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
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

void	Triangle::initVulkan( void )
{
	this->createInstance();
	this->setupDebugMessenger();
	this->createSurface();
	this->pickPhysicalDevice();
	this->createLogicalDevice();
	this->createSwapChain();
}

void	Triangle::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = this->querySwapChainSupport(this->_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = this->chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = this->chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = this->chooseSwapExtent(swapChainSupport.capabilities);

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
	QueueFamilyIndices indices = findQueueFamilies(this->_physicalDevice);
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

	if (vkCreateSwapchainKHR(this->_vkDevice, &createInfo, nullptr, &this->_swapChain) != VK_SUCCESS)
		throw std::runtime_error("échec de la création de la swap chain!");

	vkGetSwapchainImagesKHR(this->_vkDevice, this->_swapChain, &imageCount, nullptr);
	this->_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(this->_vkDevice, this->_swapChain, &imageCount, this->_swapChainImages.data());
	this->_swapChainImageFormat = surfaceFormat.format;
	this->_swapChainExtent = extent;
}

void	Triangle::createSurface()
{
	if (glfwCreateWindowSurface(this->_instance, this->_window, nullptr, &this->_surface) != VK_SUCCESS)
		throw std::runtime_error("échec de la création de la window surface!");
}

void	Triangle::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(this->_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

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

	createInfo.enabledExtensionCount = static_cast<uint32_t>(this->_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = this->_deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}
	if (vkCreateDevice(this->_physicalDevice, &createInfo, nullptr, &this->_vkDevice) != VK_SUCCESS) {
		throw std::runtime_error("échec lors de la création d'un logical device!");
	}
	vkGetDeviceQueue(this->_vkDevice, indices.graphicsFamily.value(), 0, &this->_graphicsQueue); //0 est l'index, qu'une queue ici donc juste 0
	vkGetDeviceQueue(this->_vkDevice, indices.presentFamily.value(), 0, &this->_presentQueue);
}

void	Triangle::pickPhysicalDevice( void )
{
	this->_physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, nullptr);
	if (deviceCount == 0) //si pas de carte graphique rien ne sert de lancer
		throw std::runtime_error("aucune carte graphique ne supporte Vulkan!");

	this->_devices.resize(deviceCount);
	vkEnumeratePhysicalDevices(this->_instance, &deviceCount, this->_devices.data());
	//selection des cg

	for (const auto& device : this->_devices) {
		if (isDeviceSuitable(device)) {
			this->_physicalDevice = device;
			break;
		}
	}

	// L'utilisation d'une map permet de les trier automatiquement de manière ascendante
	std::multimap<int, VkPhysicalDevice>	candidates;

	for (const auto& device : this->_devices) {
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Voyons si la meilleure possède les fonctionnalités dont nous ne pouvons nous passer
	if (candidates.rbegin()->first > 0) {
		this->_physicalDevice = candidates.rbegin()->second;
	} else {
		throw std::runtime_error("aucun GPU ne peut executer ce programme!");
	}

	if (this->_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("aucun GPU ne peut exécuter ce programme!");
	}
}

int	Triangle::rateDeviceSuitability(VkPhysicalDevice device)
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
bool	Triangle::isDeviceSuitable(VkPhysicalDevice device)
{
	this->_deviceExtensions.push_back(
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
			);

	bool	extensionsSupported = checkDeviceExtensionSupport(device);

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
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

VkSurfaceFormatKHR	Triangle::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	//si ca foire, on pourrait tester d'autres formats un peux moins bien mais dans un soucis de simplicite, on prend le premier venu
	return availableFormats[0];
}

VkPresentModeKHR	Triangle::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes)
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	//si on trouve pas le triple buffering (le meilleur atm), on pourrait en viser un autre mais son prend le mode par default, toujours present, la V-Sync
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D	Triangle::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = {WIDTH, HEIGHT};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height); //clamp comme en css pour la taille min et max

		return actualExtent;
	}
}

bool	Triangle::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t	extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(this->_deviceExtensions.begin(), this->_deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails	Triangle::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails		details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->_surface, &details.capabilities);

	uint32_t	formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->_surface, &formatCount, nullptr);

	if (formatCount != 0) {
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

QueueFamilyIndices	Triangle::findQueueFamilies(VkPhysicalDevice device) {
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

void Triangle::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void Triangle::setupDebugMessenger() {
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(this->_instance, &createInfo, nullptr, &this->_debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("failed to set up debug messenger!");
}

void Triangle::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("validation layers requested, but not available!");

	VkApplicationInfo appInfo{}; // informations optionnelles mais utiles pour optimiser
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // expliciter le type
	appInfo.pApplicationName = "Hello Triangle"; // nom de l'app
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // version
	appInfo.pEngineName = "No Engine"; // engine si utilise
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{}; // structure permettant la création de l'instance. Celle-ci n'est pas optionnelle. -> informer le driver des extensions et des validation layers
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
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

bool	Triangle::checkValidationLayerSupport()
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
		{
			return false;
		}
	}
	return true;
}

std::vector<const char*>	Triangle::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void	Triangle::mainLoop( void )
{
	while (!glfwWindowShouldClose(this->_window) && !QUIT) {
		glfwSetKeyCallback(this->_window, key_callback);
		glfwPollEvents();
	}
}

void	Triangle::cleanup( void )
{
	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(this->_instance, this->_debugMessenger, nullptr);
	vkDestroySwapchainKHR(this->_vkDevice, this->_swapChain, nullptr);
	vkDestroyDevice(this->_vkDevice, nullptr);
	vkDestroySurfaceKHR(this->_instance, this->_surface, nullptr);
	vkDestroyInstance(this->_instance, nullptr);
	glfwDestroyWindow(this->_window);
	glfwTerminate();
}
