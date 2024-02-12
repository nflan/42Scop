/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 17:48:11 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 17:48:11 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Device.hpp"
#include "Window.hpp"
#include "SwapChain.hpp"

#ifndef DEBUG
#define NDEBUG
#endif
#include <array>
#include <cassert>
#include <stdexcept>
#include <memory>
#include <vector>

class   ft_Renderer {
    public:
        ft_Renderer(ft_Window &window, ft_Device &device);
        ~ft_Renderer();

        ft_Renderer(const ft_Renderer &) = delete;
        ft_Renderer &operator=(const ft_Renderer &) = delete;

        VkRenderPass    getSwapChainRenderPass() const { return _swapChain->getRenderPass(); }
        float           getAspectRatio() const { return _swapChain->extentAspectRatio(); }
        ft_SwapChain&   getSwapChain() const { return *_swapChain; }
        bool            isFrameInProgress() const { return _isFrameStarted; }
        ft_Window&      getWindow() { return _window; }
        ft_Device&      getDevice() { return _device; }
        
        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(_isFrameStarted && "Cannot get command buffer when frame not in progress");
            return _commandBuffers[_currentFrameIndex];
        }

        int             getFrameIndex() const {
            assert(_isFrameStarted && "Cannot get frame index when frame not in progress");
            return _currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void            endFrame();
        void            beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void            endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        ft_Window&                      _window;
        ft_Device&                      _device;
        std::unique_ptr<ft_SwapChain>   _swapChain;
        std::vector<VkCommandBuffer>    _commandBuffers;

        uint32_t                        _currentImageIndex;
        int                             _currentFrameIndex{0};
        bool                            _isFrameStarted{false};
};

#endif