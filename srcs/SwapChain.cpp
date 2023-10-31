/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SwapChain.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 17:56:06 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 17:56:08 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/SwapChain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

ft_SwapChain::ft_SwapChain(ft_Device &deviceRef, VkExtent2D extent): _device{deviceRef}, _windowExtent{extent}
{
    init();
}

ft_SwapChain::ft_SwapChain(ft_Device &deviceRef, VkExtent2D extent, std::shared_ptr<ft_SwapChain> previous): _device{deviceRef}, _windowExtent{extent}, _oldSwapChain{previous}
{
    init();
    _oldSwapChain = nullptr;
}

void ft_SwapChain::init()
{
    createSwapChain();
    createImageViews();
    createRenderPass();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

ft_SwapChain::~ft_SwapChain()
{
    for (auto imageView : this->_swapChainImageViews) {
        vkDestroyImageView(this->_device.device(), imageView, nullptr);
    }
    this->_swapChainImageViews.clear();

    if (this->_swapChain != nullptr) {
        vkDestroySwapchainKHR(this->_device.device(), this->_swapChain, nullptr);
        this->_swapChain = nullptr;
    }

    for (int i = 0; i < this->_depthImages.size(); i++) {
        vkDestroyImageView(this->_device.device(), this->_depthImageViews[i], nullptr);
        vkDestroyImage(this->_device.device(), this->_depthImages[i], nullptr);
        vkFreeMemory(this->_device.device(), this->_depthImageMemorys[i], nullptr);
    }

    for (int i = 0; i < this->_colorImages.size(); i++)
    {
    	vkDestroyImageView(this->_device.device(), this->_colorImageViews[i], nullptr);
        vkDestroyImage(this->_device.device(), this->_colorImages[i], nullptr);
        vkFreeMemory(this->_device.device(), this->_colorImageMemorys[i], nullptr);
    }

    for (auto framebuffer : this->_swapChainFramebuffers) {
        vkDestroyFramebuffer(this->_device.device(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(this->_device.device(), this->_renderPass, nullptr);

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(this->_device.device(), this->_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(this->_device.device(), this->_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(this->_device.device(), this->_inFlightFences[i], nullptr);
    }
}

VkResult    ft_SwapChain::acquireNextImage(uint32_t *imageIndex)
{
    vkWaitForFences(
        this->_device.device(),
        1,
        &this->_inFlightFences[this->_currentFrame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(
        this->_device.device(),
        this->_swapChain,
        std::numeric_limits<uint64_t>::max(),
        this->_imageAvailableSemaphores[this->_currentFrame],  // must be a not signaled semaphore
        VK_NULL_HANDLE,
        imageIndex);

    return result;
}

VkResult    ft_SwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex)
{
    if (this->_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(this->_device.device(), 1, &this->_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    this->_imagesInFlight[*imageIndex] = this->_inFlightFences[this->_currentFrame];

    VkSubmitInfo    submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { this->_imageAvailableSemaphores[this->_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {this->_renderFinishedSemaphores[this->_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(this->_device.device(), 1, &this->_inFlightFences[this->_currentFrame]);
    if (vkQueueSubmit(this->_device.graphicsQueue(), 1, &submitInfo, this->_inFlightFences[this->_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR    presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR  swapChains[] = {this->_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR(this->_device.presentQueue(), &presentInfo);

    this->_currentFrame = (this->_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void    ft_SwapChain::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = this->_device.getSwapChainSupport();

    VkSurfaceFormatKHR      surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR        presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D              extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t    imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->_device.surface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	/*
	 * Le champ de bits imageUsage spécifie le type d'opérations que nous appliquerons aux images de la swap chain. Dans ce tutoriel nous effectuerons un rendu directement sur les images, nous les utiliserons donc comme color attachement. Vous voudrez peut-être travailler sur une image séparée pour pouvoir appliquer des effets en post-processing. Dans ce cas vous devrez utiliser une valeur comme VK_IMAGE_USAGE_TRANSFER_DST_BIT à la place et utiliser une opération de transfert de mémoire pour placer le résultat final dans une image de la swap chain.
	*/
    QueueFamilyIndices  indices = this->_device.findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

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

    createInfo.oldSwapchain = this->_oldSwapChain == nullptr ? VK_NULL_HANDLE : this->_oldSwapChain->_swapChain; // si la swap chain crash (resize par exemple), la nouvelle doit envoyer un pointer sur la precedente

    if (vkCreateSwapchainKHR(this->_device.device(), &createInfo, nullptr, &this->_swapChain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(this->_device.device(), this->_swapChain, &imageCount, nullptr);
    this->_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->_device.device(), this->_swapChain, &imageCount, this->_swapChainImages.data());

    this->_swapChainImageFormat = surfaceFormat.format;
    this->_swapChainExtent = extent;
}

VkImageView ft_SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView	imageView;
    if (vkCreateImageView(this->_device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("échec de la creation de la vue sur une image!");

    return imageView;
}

void    ft_SwapChain::createImageViews()
{
    this->_swapChainImageViews.resize(this->_swapChainImages.size());

    for (uint32_t i = 0; i < this->_swapChainImages.size(); i++)
        this->_swapChainImageViews[i] = this->createImageView(this->_swapChainImages[i], this->_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void    ft_SwapChain::createRenderPass()
{
    //ATTACHEMENTS
    //Depth Attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
 	depthAttachment.samples = this->_device.getMsaaSamples();
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //Color Attachment
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = getSwapChainImageFormat();
	colorAttachment.samples = this->_device.getMsaaSamples();
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //action de l'attachement avant rendu (enum)
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // action de l'attachement apres rendu (enum)
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//stencil? on n'utilise pas
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//stencil? on n'utilise pas
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = this->_swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//SUBPASSES ET REFERENCES AUX ATTACHEMENTS
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference   depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

	//Subpass dependencies
    VkSubpassDependency dependency = {};
	dependency.dstSubpass = 0; // dst doit etre > src sauf si VK_SUBPASS_EXTERNAL (vu que c'est avant la 0)
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	//PASSE DE RENDU
	std::array<VkAttachmentDescription, 3>  attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    
    VkRenderPassCreateInfo  renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(this->_device.device(), &renderPassInfo, nullptr, &this->_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void    ft_SwapChain::createFramebuffers()
{
    this->_swapChainFramebuffers.resize(imageCount());
    for (size_t i = 0; i < imageCount(); i++) {
		std::array<VkImageView, 3> attachments = {
			this->_colorImageViews[i],
    		this->_depthImageViews[i],
    		this->_swapChainImageViews[i]
		};

        VkExtent2D swapChainExtent = getSwapChainExtent();
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = this->_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //car une seule couche dans la swap chain

        if (vkCreateFramebuffer(this->_device.device(), &framebufferInfo, nullptr, &this->_swapChainFramebuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void    ft_SwapChain::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo	imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	//1D = utilisé comme des tableaux ou des gradients
	//2D = majoritairement utilisés comme textures
	//3D = utilisées pour stocker des voxels par exemple
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format; //meme que dans celles dans le buffer
	imageInfo.tiling = tiling;
	//VK_IMAGE_TILING_LINEAR : les texels sont organisés ligne par ligne
	//VK_IMAGE_TILING_OPTIMAL : les texels sont organisés de la manière la plus optimale pour l'implémentation
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//VK_IMAGE_LAYOUT_UNDEFINED : inutilisable par le GPU, son contenu sera éliminé à la première transition
	//VK_IMAGE_LAYOUT_PREINITIALIZED : inutilisable par le GPU, mais la première transition conservera les texels
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;
	imageInfo.flags = 0; // Optionnel
	//Ces image étendues sont des images dont seule une partie est stockée dans la mémoire. Voici une exemple d'utilisation : si vous utilisiez une image 3D pour représenter un terrain à l'aide de voxels, vous pourriez utiliser cette fonctionnalité pour éviter d'utiliser de la mémoire qui au final ne contiendrait que de l'air. Nous ne verrons pas cette fonctionnalité dans ce tutoriel, donnez à flags la valeur 0.

    if (vkCreateImage(this->_device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("echec de la creation d'une image!");

    VkMemoryRequirements	memRequirements;
    vkGetImageMemoryRequirements(this->_device.device(), image, &memRequirements);

    VkMemoryAllocateInfo	allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = this->_device.findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->_device.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");

    vkBindImageMemory(this->_device.device(), image, imageMemory, 0);
}

void    ft_SwapChain::createDepthResources()
{
    VkFormat    depthFormat = findDepthFormat();
    this->_swapChainDepthFormat = depthFormat;
    VkExtent2D  swapChainExtent = getSwapChainExtent();

    this->_depthImages.resize(imageCount());
    this->_depthImageMemorys.resize(imageCount());
    this->_depthImageViews.resize(imageCount());

    for (int i = 0; i < this->_depthImages.size(); i++)
    {
        VkImageCreateInfo	imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = this->_device.getMsaaSamples();
        imageInfo.flags = 0; // Optionnel

        this->_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_depthImages[i], this->_depthImageMemorys[i]);

	    this->_depthImageViews[i] = this->createImageView(this->_depthImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }
}

void	ft_SwapChain::createColorResources()
{
	VkFormat	colorFormat = this->_swapChainImageFormat;
    VkExtent2D  swapChainExtent = getSwapChainExtent();

    this->_colorImages.resize(imageCount());
    this->_colorImageMemorys.resize(imageCount());
    this->_colorImageViews.resize(imageCount());

    for (int i = 0; i < this->_colorImages.size(); i++)
    {
        VkImageCreateInfo	imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = colorFormat; //meme que dans celles dans le buffer
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = this->_device.getMsaaSamples();
        imageInfo.flags = 0; // Optionnel

        this->_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->_colorImages[i], this->_colorImageMemorys[i]);

	    this->_colorImageViews[i] = this->createImageView(this->_colorImages[i], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void    ft_SwapChain::createSyncObjects()
{
    this->_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    this->_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo   semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo   fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
		if (vkCreateSemaphore(this->_device.device(), &semaphoreInfo, nullptr, &this->_imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(this->_device.device(), &semaphoreInfo, nullptr, &this->_renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(this->_device.device(), &fenceInfo, nullptr, &this->_inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

VkSurfaceFormatKHR  ft_SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for (const auto& availableFormat : availableFormats)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	//si ca foire, on pourrait tester d'autres formats un peux moins bien mais dans un soucis de simplicite, on prend le premier venu
	return availableFormats[0];
}

VkPresentModeKHR    ft_SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            std::cout << "Present mode: Mailbox" << std::endl;
            return availablePresentMode;
        }
    }

    // for (const auto &availablePresentMode : availablePresentModes) {
    //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     std::cout << "Present mode: Immediate" << std::endl;
    //     return availablePresentMode;
    //   }
    // }

    std::cout << "Present mode: V-Sync" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ft_SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    else
    {
        /*
        		int	width, height;
		glfwGetFramebufferSize(this->_window.getWindow(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height); //clamp comme en css pour la taille min et max
        */
        VkExtent2D actualExtent = this->_windowExtent;
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkFormat ft_SwapChain::findDepthFormat()
{
    return this->_device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}