/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Device.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 15:22:06 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 15:22:07 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Window.hpp"
#include <vector>
#include <string>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

class ft_Device {
    public:
        ft_Device(ft_Window &window);
        ~ft_Device();

        // Not copyable or movable
        ft_Device(const ft_Device &) = delete;
        ft_Device &operator=(const ft_Device &) = delete;
        ft_Device(ft_Device &&) = delete;
        ft_Device &operator=(ft_Device &&) = delete;

        VkCommandPool                   getCommandPool() { return _commandPool; }
        VkDevice                        device() { return _device_; }
        VkDevice*                       pDevice() { return &_device_; }
        VkSurfaceKHR                    surface() { return _surface_; }
        VkQueue                         graphicsQueue() { return _graphicsQueue_; }
        VkQueue                         presentQueue() { return _presentQueue_; }
        VkSampleCountFlagBits           getMsaaSamples() { return _msaaSamples; }
        VkPhysicalDevice                getPhysicalDevice() { return _physicalDevice; }
        VkQueue                         getGraphicQueue() { return _graphicsQueue_; }
        VkQueue*                        getpGraphicQueue() { return &_graphicsQueue_; }
        VkQueue                         getPresentQueue() { return _presentQueue_; }
        VkQueue*                        getpPresentQueue() { return &_presentQueue_; }
        VkSurfaceKHR                    getSurface() { return _surface_; }
        ft_Window&                      getWindow() { return _window; }
        const std::vector<const char *> getValidationLayers() { return _validationLayers; }
        const std::vector<const char *> getDeviceExtensions() { return _deviceExtensions; }

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(_physicalDevice); }
        // void                    setMipLevels(uint32_t m) { _mipLevels = m; }
        // uint32_t                getMipLevels() { return _mipLevels; }
        uint32_t                findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices      findPhysicalQueueFamilies() { return findQueueFamilies(_physicalDevice); }
        VkFormat                findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat                findDepthFormat();

        // Buffer Helper Functions
        void                    createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
        VkCommandBuffer         beginSingleTimeCommands();
        void                    endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void                    copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void                    copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

        VkSampleCountFlagBits   getMaxUsableSampleCount(VkSampleCountFlags requestedSampleCount);
        int                     rateDeviceSuitability(VkPhysicalDevice device);
        void                    createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

        VkPhysicalDeviceProperties properties;

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice device);

    private:
        void    createInstance();
        void    setupDebugMessenger();
        void    createSurface();
        void    pickPhysicalDevice();
        void    createLogicalDevice();
        void    createCommandPool();

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char *> getRequiredExtensions();
        bool checkValidationLayerSupport();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        VkInstance                      _instance;
        VkDebugUtilsMessengerEXT        _debugMessenger;
        VkPhysicalDevice                _physicalDevice = VK_NULL_HANDLE;
        ft_Window&                      _window;
        VkSampleCountFlagBits		    _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        // uint32_t					    _mipLevels;
        VkCommandPool                   _commandPool;

        VkDevice                        _device_;
        VkSurfaceKHR                    _surface_;
        VkQueue                         _graphicsQueue_;
        VkQueue                         _presentQueue_;

        const std::vector<const char *> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

#endif
