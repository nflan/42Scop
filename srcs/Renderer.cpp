/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Renderer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 17:48:53 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 17:48:54 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Renderer.hpp"

#include <array>
#include <cassert>
#include <stdexcept>

ft_Renderer::ft_Renderer(ft_Window& window, ft_Device& device): _window{window}, _device{device}
{
    recreateSwapChain();
    createCommandBuffers();
}

ft_Renderer::~ft_Renderer()
{
    freeCommandBuffers();
}

void    ft_Renderer::recreateSwapChain()
{
    auto extent = this->_window.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = this->_window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(this->_device.device());

    if (this->_swapChain == nullptr)
    {
        this->_swapChain = std::make_unique<ft_SwapChain>(this->_device, extent);
    }
    else
    {
        std::shared_ptr<ft_SwapChain> oldSwapChain = std::move(this->_swapChain);
        this->_swapChain = std::make_unique<ft_SwapChain>(this->_device, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormats(*this->_swapChain.get()))
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
    }
}

void    ft_Renderer::createCommandBuffers()
{
    this->_commandBuffers.resize(ft_SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    //VK_COMMAND_BUFFER_LEVEL_PRIMARY : peut être envoyé à une queue pour y être exécuté mais ne peut être appelé par d'autres command buffers
	//VK_COMMAND_BUFFER_LEVEL_SECONDARY : ne peut pas être directement émis à une queue mais peut être appelé par un autre command buffer
    allocInfo.commandPool = this->_device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(this->_commandBuffers.size());

    if (vkAllocateCommandBuffers(this->_device.device(), &allocInfo, this->_commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}

void    ft_Renderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(this->_device.device(), this->_device.getCommandPool(), static_cast<uint32_t>(this->_commandBuffers.size()), this->_commandBuffers.data());
    this->_commandBuffers.clear();
}

VkCommandBuffer ft_Renderer::beginFrame()
{
    assert(!this->_isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = this->_swapChain->acquireNextImage(&this->_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    this->_isFrameStarted = true;

    auto    commandBuffer = this->getCurrentCommandBuffer();
    VkCommandBufferBeginInfo    beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void ft_Renderer::endFrame()
{
    assert(this->_isFrameStarted && "Can't call endFrame while frame is not in progress");

    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");

    auto result = this->_swapChain->submitCommandBuffers(&commandBuffer, &this->_currentImageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->_window.wasWindowResize())
    {
        this->_window.resetWindowResizedFlag();
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");

    this->_isFrameStarted = false;
    this->_currentFrameIndex = (this->_currentFrameIndex + 1) % ft_SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void    ft_Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(this->_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

	//Commencer une render pass
    VkRenderPassBeginInfo   renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->_swapChain->getRenderPass();
    renderPassInfo.framebuffer = this->_swapChain->getFrameBuffer(this->_currentImageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->_swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
	clearValues[1].depthStencil = {1.0f, 0};//Dans vulkan, 0.0 correspond au plan near et 1.0 far.

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;// pour inverse viewport, mettre la height (a faire au debut du projet ptete)
    viewport.width = static_cast<float>(this->_swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(this->_swapChain->getSwapChainExtent().height); // on peut inverser l'axe dans le viewport mais ca inverse tous les autres calculs et ca trigger les validations layers
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, this->_swapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void ft_Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(this->_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
}