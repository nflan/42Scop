/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SwapChain.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 17:56:14 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 17:56:16 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include "Device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

class ft_SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        ft_SwapChain(ft_Device &deviceRef, VkExtent2D windowExtent);
        ft_SwapChain(ft_Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<ft_SwapChain> previous);
        
        ~ft_SwapChain();

        ft_SwapChain(const ft_SwapChain &) = delete;
        ft_SwapChain &operator=(const ft_SwapChain &) = delete;

        VkFramebuffer   getFrameBuffer(int index) { return _swapChainFramebuffers[index]; }
        VkRenderPass    getRenderPass() { return _renderPass; }
        VkImageView     getImageView(int index) { return _swapChainImageViews[index]; }
        size_t          imageCount() { return _swapChainImages.size(); }
        VkFormat        getSwapChainImageFormat() { return _swapChainImageFormat; }
        VkExtent2D      getSwapChainExtent() { return _swapChainExtent; }
        uint32_t        getWidth() { return _swapChainExtent.width; }
        uint32_t        getHeight() { return _swapChainExtent.height; }
        ft_Device&      getDevice() { return _device; }

        float           extentAspectRatio()
        {
            return static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height);
        }
        VkFormat        findDepthFormat();

        VkResult        acquireNextImage(uint32_t *imageIndex);
        VkResult        submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool            compareSwapFormats(const ft_SwapChain &swapChain) const
        {
            return swapChain._swapChainDepthFormat == _swapChainDepthFormat &&
                swapChain._swapChainImageFormat == _swapChainImageFormat;
        }

        void                createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    private:
        void    init();
        void    createSwapChain();
        void    createImageViews();
        void    createDepthResources();
        void	createColorResources();
        void    createRenderPass();
        void    createFramebuffers();
        void    createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR  chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR    chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D          chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkSwapchainKHR              _swapChain;
        std::shared_ptr<ft_SwapChain>   _oldSwapChain;
        VkFormat                    _swapChainImageFormat;
        VkFormat                    _swapChainDepthFormat;
        VkExtent2D                  _swapChainExtent;

        std::vector<VkFramebuffer>  _swapChainFramebuffers;
        VkRenderPass                _renderPass;

        std::vector<VkImage>        _depthImages;
        std::vector<VkDeviceMemory> _depthImageMemorys;
        std::vector<VkImageView>    _depthImageViews;
        std::vector<VkImage>        _swapChainImages;
        std::vector<VkImageView>    _swapChainImageViews;

        ft_Device&                  _device;
        VkExtent2D                  _windowExtent;

        std::vector<VkSemaphore>    _imageAvailableSemaphores;
        std::vector<VkSemaphore>    _renderFinishedSemaphores;
        std::vector<VkFence>        _inFlightFences;
        std::vector<VkFence>        _imagesInFlight;
        size_t                      _currentFrame = 0;

        //Multisampling -> anti-aliasing
		std::vector<VkImage>		_colorImages;
		std::vector<VkDeviceMemory>	_colorImageMemorys;
		std::vector<VkImageView>	_colorImageViews;
};

#endif