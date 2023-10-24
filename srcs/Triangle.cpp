/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Triangle.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:10:26 by nflan             #+#    #+#             */
/*   Updated: 2023/10/24 19:10:21 by nflan            ###   ########.fr       */
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
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

bool	QUIT = false;

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

void	Triangle::initVulkan( void ) {
	this->createInstance();
	this->setupDebugMessenger();
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void Triangle::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(this->_instance, &createInfo, nullptr, &this->_debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void Triangle::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("les validations layers sont activées mais ne sont pas disponibles!");
	}
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

	uint32_t		glfwExtensionCount = 0;
	const char**	glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount = 0;

	VkDebugUtilsMessengerEXT callback; // fournir la fonction a Vulkan
	/*	uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::cout << "Extensions disponibles :\n";

		for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
		}
		*/
	std::cout << "j'init vulkan" << std::endl;
	if (vkCreateInstance(&createInfo, nullptr, &this->_instance) != VK_SUCCESS) {
		throw std::runtime_error("Echec de la création de l'instance!");
	}
	//Pointeur sur une structure contenant l'information pour la création
	//Pointeur sur une fonction d'allocation que nous laisserons toujours nullptr
	//Pointeur sur une variable stockant une référence au nouvel objet
}

bool	Triangle::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
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

std::vector<const char*>	Triangle::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void	Triangle::mainLoop( void ) {
	while (!glfwWindowShouldClose(this->_window) && !QUIT) {
		glfwSetKeyCallback(this->_window, key_callback);
		glfwPollEvents();
	}
};

void	Triangle::cleanup( void ) {
	vkDestroyInstance(this->_instance, nullptr);
	glfwDestroyWindow(this->_window);
	glfwTerminate();
};
