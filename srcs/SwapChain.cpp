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

ft_SwapChain::ft_SwapChain(ft_Device &deviceRef, VkExtent2D extent): _device{deviceRef}, _windowExtent{extent}
{
    init();
}

ft_SwapChain::ft_SwapChain(ft_Device &deviceRef, VkExtent2D extent, std::shared_ptr<ft_SwapChain> previous): _device{deviceRef}, _windowExtent{extent}, _oldSwapChain{previous}
{
    _swapChain = nullptr;
    _renderPass = nullptr;
    init();
}

void ft_SwapChain::init()
{
    char    status = 0;
    createSwapChain();
    try {
        createImageViews();
        ++status;
        this->_swapChainDepthFormat = this->findDepthFormat();
        createRenderPass();
        ++status;
        createColorResources();
        ++status;
        createDepthResources();
        ++status;
        createFramebuffers();
        ++status;
        createSyncObjects();
    }
    catch (const std::exception& e) {
        if (status) {
            for (VkImageView imageView : this->_swapChainImageViews) {
                vkDestroyImageView(this->_device.device(), imageView, nullptr);
            }
            this->_swapChainImageViews.clear();
        }

        if (this->_swapChain != nullptr) {
            vkDestroySwapchainKHR(this->_device.device(), this->_swapChain, nullptr);
            this->_swapChain = nullptr;
        }

        if (status > 3) {
            for (std::size_t i = 0; i < this->_depthImages.size(); i++) {
                vkDestroyImageView(this->_device.device(), this->_depthImageViews[i], nullptr);
                vkDestroyImage(this->_device.device(), this->_depthImages[i], nullptr);
                vkFreeMemory(this->_device.device(), this->_depthImageMemorys[i], nullptr);
            }
        }

        if (status > 2) {
            for (std::size_t i = 0; i < this->_colorImages.size(); i++) {
                vkDestroyImageView(this->_device.device(), this->_colorImageViews[i], nullptr);
                vkDestroyImage(this->_device.device(), this->_colorImages[i], nullptr);
                vkFreeMemory(this->_device.device(), this->_colorImageMemorys[i], nullptr);
            }
        }

        if (status > 4) {
            for (VkFramebuffer& framebuffer : this->_swapChainFramebuffers) {
                vkDestroyFramebuffer(this->_device.device(), framebuffer, nullptr);
            }
        }

        if (status > 1)
        vkDestroyRenderPass(this->_device.device(), this->_renderPass, nullptr);

        throw std::runtime_error(e.what());
    }
}

ft_SwapChain::~ft_SwapChain()
{
    for (VkImageView imageView : this->_swapChainImageViews) {
        vkDestroyImageView(this->_device.device(), imageView, nullptr);
    }
    this->_swapChainImageViews.clear();

    if (this->_swapChain != nullptr) {
        vkDestroySwapchainKHR(this->_device.device(), this->_swapChain, nullptr);
        this->_swapChain = nullptr;
    }

    for (std::size_t i = 0; i < this->_depthImages.size(); i++) {
        vkDestroyImageView(this->_device.device(), this->_depthImageViews[i], nullptr);
        vkDestroyImage(this->_device.device(), this->_depthImages[i], nullptr);
        vkFreeMemory(this->_device.device(), this->_depthImageMemorys[i], nullptr);
    }

    for (std::size_t i = 0; i < this->_colorImages.size(); i++) {
    	vkDestroyImageView(this->_device.device(), this->_colorImageViews[i], nullptr);
        vkDestroyImage(this->_device.device(), this->_colorImages[i], nullptr);
        vkFreeMemory(this->_device.device(), this->_colorImageMemorys[i], nullptr);
    }

    for (VkFramebuffer framebuffer : this->_swapChainFramebuffers) {
        vkDestroyFramebuffer(this->_device.device(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(this->_device.device(), this->_renderPass, nullptr);

    // cleanup synchronization objects
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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
        std::numeric_limits<uint32_t>::max());

    VkResult result = vkAcquireNextImageKHR(
        this->_device.device(),
        this->_swapChain,
        std::numeric_limits<uint32_t>::max(),
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

    VkSemaphore waitSemaphores[] = { this->_imageAvailableSemaphores[this->_currentFrame]};
    VkPipelineStageFlags    waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {this->_renderFinishedSemaphores[this->_currentFrame]};
    VkSubmitInfo    submitInfo {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .waitSemaphoreCount = 1,
                                .pWaitSemaphores = waitSemaphores,
                                .pWaitDstStageMask = waitStages,
                                .commandBufferCount = 1,
                                .pCommandBuffers = buffers,
                                .signalSemaphoreCount = 1,
                                .pSignalSemaphores = signalSemaphores};

    vkResetFences(this->_device.device(), 1, &this->_inFlightFences[this->_currentFrame]);
    if (vkQueueSubmit(this->_device.graphicsQueue(), 1, &submitInfo, this->_inFlightFences[this->_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkSwapchainKHR  swapChains[] = {this->_swapChain};
    VkPresentInfoKHR    presentInfo {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                     .waitSemaphoreCount = 1,
                                     .pWaitSemaphores = signalSemaphores,
                                     .swapchainCount = 1,
                                     .pSwapchains = swapChains,
                                     .pImageIndices = imageIndex};
    this->_currentFrame += 1;
    if (this->_currentFrame >= MAX_FRAMES_IN_FLIGHT)
        this->_currentFrame = 0;

    return vkQueuePresentKHR(this->_device.presentQueue(), &presentInfo);
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

    VkSwapchainCreateInfoKHR    createInfo = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                                .surface = this->_device.surface(),
                                                .minImageCount = imageCount,
                                                .imageFormat = surfaceFormat.format,
                                                .imageColorSpace = surfaceFormat.colorSpace,
                                                .imageExtent = extent,
                                                .imageArrayLayers = 1,
                                                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                                .queueFamilyIndexCount = 0,
                                                .pQueueFamilyIndices = nullptr,
                                                .preTransform = swapChainSupport.capabilities.currentTransform,// si on veut transform genre rotate ou symetrie verticale
                                                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,//si on veut que la fenetre influe sur les couleurs de l'image (generalement non, comme ici)
                                                .presentMode = presentMode,// meilleures performances avec clipped = vk_true
                                                .clipped = VK_TRUE,//pas afficher pixels derrieres
                                                .oldSwapchain = this->_oldSwapChain == nullptr ? VK_NULL_HANDLE : this->_oldSwapChain->_swapChain}; // si la swap chain crash (resize par exemple), la nouvelle doit envoyer un pointer sur la precedente
	/*
	 * Le champ de bits imageUsage spécifie le type d'opérations que nous appliquerons aux images de la swap chain. Dans ce tutoriel nous effectuerons un rendu directement sur les images, nous les utiliserons donc comme color attachement. Vous voudrez peut-être travailler sur une image séparée pour pouvoir appliquer des effets en post-processing. Dans ce cas vous devrez utiliser une valeur comme VK_IMAGE_USAGE_TRANSFER_DST_BIT à la place et utiliser une opération de transfert de mémoire pour placer le résultat final dans une image de la swap chain.
	*/
    QueueFamilyIndices  indices = this->_device.findPhysicalQueueFamilies();
    uint32_t            queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // pouvoir travailler a plusieurs queues (presentation et graphique) sur la meme image (moins performant)
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

    if (vkCreateSwapchainKHR(this->_device.device(), &createInfo, nullptr, &this->_swapChain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

    vkGetSwapchainImagesKHR(this->_device.device(), this->_swapChain, &imageCount, nullptr);
    this->_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->_device.device(), this->_swapChain, &imageCount, this->_swapChainImages.data());

    this->_swapChainImageFormat = surfaceFormat.format;
    this->_swapChainExtent = extent;
}

VkImageView ft_SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo   viewInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                        .image = image,
                                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                        .format = format,
                                        .subresourceRange{.aspectMask = aspectFlags,
                                                            .baseMipLevel = 0,
                                                            .levelCount = mipLevels,
                                                            .baseArrayLayer = 0,
                                                            .layerCount = 1}
                                        };
    VkImageView imageView;
    if (vkCreateImageView(this->_device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("échec de la creation de la vue sur une image!");

    return imageView;
}

void    ft_SwapChain::createImageViews()
{
    this->_swapChainImageViews.resize(this->_swapChainImages.size());

    for (std::size_t i = 0; i < this->_swapChainImages.size(); i++) {
        this->_swapChainImageViews[i] = this->createImageView(this->_swapChainImages[i], this->_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void    ft_SwapChain::createRenderPass()
{
    //ATTACHEMENTS
    //Color Attachment
    VkAttachmentDescription colorAttachment = {.format = this->_swapChainImageFormat,
                                                .samples = this->_device.getMsaaSamples(),
                                                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    //Depth Attachment
    VkAttachmentDescription depthAttachment{.format = this->_swapChainDepthFormat,
                                            .samples = this->_device.getMsaaSamples(),
                                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription colorAttachmentResolve{.format = this->_swapChainImageFormat,
                                            .samples = VK_SAMPLE_COUNT_1_BIT,
                                            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

	//SUBPASSES ET REFERENCES AUX ATTACHEMENTS
	VkAttachmentReference  colorAttachmentRef{.attachment = 0,
                                                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    
    VkAttachmentReference   depthAttachmentRef{.attachment = 1,
                                                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

	VkAttachmentReference   colorAttachmentResolveRef{.attachment = 2,
                                                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription    subpass = {.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        .colorAttachmentCount = 1,
                                        .pColorAttachments = &colorAttachmentRef,
                                        .pResolveAttachments = &colorAttachmentResolveRef,
                                        .pDepthStencilAttachment = &depthAttachmentRef};

	//Subpass dependencies
    VkSubpassDependency dependency = {.srcSubpass = VK_SUBPASS_EXTERNAL,
                                        .dstSubpass = 0, // dst doit etre > src sauf si VK_SUBPASS_EXTERNAL
                                        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};

    //PASSE DE RENDU
	std::array<VkAttachmentDescription, 3>  attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    
    VkRenderPassCreateInfo  renderPassInfo = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                                .pAttachments = attachments.data(),
                                                .subpassCount = 1,
                                                .pSubpasses = &subpass,
                                                .dependencyCount = 1,
                                                .pDependencies = &dependency};

    if (vkCreateRenderPass(this->_device.device(), &renderPassInfo, nullptr, &this->_renderPass) != VK_SUCCESS)
        throw std::runtime_error("failed to create render pass!");
}

void    ft_SwapChain::createFramebuffers()
{
    this->_swapChainFramebuffers.resize(_swapChainImageViews.size());
    for (std::size_t i = 0; i < _swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3>  attachments = {
			this->_colorImageViews[0],
    		this->_depthImageViews[0],
    		this->_swapChainImageViews[i]
		};

        VkExtent2D              swapChainExtent = getSwapChainExtent();
        VkFramebufferCreateInfo framebufferInfo{.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                .renderPass = this->_renderPass,
                                                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                                .pAttachments = attachments.data(),
                                                .width = swapChainExtent.width,
                                                .height = swapChainExtent.height,
                                                .layers = 1}; //car une seule couche dans la swap chain

        if (vkCreateFramebuffer(this->_device.device(), &framebufferInfo, nullptr, &this->_swapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}

void    ft_SwapChain::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo	imageInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                    .flags = 0,
                                    .imageType = VK_IMAGE_TYPE_2D,
                                    //1D = utilisé comme des tableaux ou des gradients
                                    //2D = majoritairement utilisés comme textures
                                    //3D = utilisées pour stocker des voxels par exemple
                                    .format = format, //meme que dans celles dans le buffer
                                    .extent{.width = width,
                                            .height = height,
                                            .depth = 1,
                                    },
                                    .mipLevels = mipLevels,
                                    .arrayLayers = 1,
                                    .samples = numSamples,
                                    .tiling = tiling,
                                    .usage = usage,
                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                    //VK_IMAGE_TILING_LINEAR : les texels sont organisés ligne par ligne
                                    //VK_IMAGE_TILING_OPTIMAL : les texels sont organisés de la manière la plus optimale pour l'implémentation
                                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
                                    //VK_IMAGE_LAYOUT_UNDEFINED : inutilisable par le GPU, son contenu sera éliminé à la première transition
                                    //VK_IMAGE_LAYOUT_PREINITIALIZED : inutilisable par le GPU, mais la première transition conservera les texels
                                    }; // Optionnel
	//Ces image étendues sont des images dont seule une partie est stockée dans la mémoire. Voici une exemple d'utilisation : si vous utilisiez une image 3D pour représenter un terrain à l'aide de voxels, vous pourriez utiliser cette fonctionnalité pour éviter d'utiliser de la mémoire qui au final ne contiendrait que de l'air.

    if (vkCreateImage(this->_device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("echec de la creation d'une image!");

    VkMemoryRequirements	memRequirements;
    vkGetImageMemoryRequirements(this->_device.device(), image, &memRequirements);

    try {
        VkMemoryAllocateInfo	allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                            .allocationSize = memRequirements.size,
                                            .memoryTypeIndex = this->_device.findMemoryType(memRequirements.memoryTypeBits, properties)};
        if (vkAllocateMemory(this->_device.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
        
        vkBindImageMemory(this->_device.device(), image, imageMemory, 0);
    }
    catch (const std::exception& e) {
        vkDestroyImage(this->_device.device(), image, nullptr);
        throw std::runtime_error(e.what());
    }
}

void    ft_SwapChain::createDepthResources()
{
    VkFormat    depthFormat = findDepthFormat();
    this->_swapChainDepthFormat = depthFormat;
    VkExtent2D  swapChainExtent = getSwapChainExtent();

    this->_depthImages.resize(1);
    this->_depthImageMemorys.resize(1);
    this->_depthImageViews.resize(1);

    createImage(swapChainExtent.width, swapChainExtent.height, 1, this->_device.getMsaaSamples(), depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImages[0], _depthImageMemorys[0]);
    _depthImageViews[0] = createImageView(_depthImages[0], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void	ft_SwapChain::createColorResources()
{
	VkFormat	colorFormat = this->_swapChainImageFormat;
    VkExtent2D  swapChainExtent = getSwapChainExtent();

    this->_colorImages.insert(this->_colorImages.begin(), 1, nullptr);
    this->_colorImageMemorys.resize(1);
    this->_colorImageViews.resize(1);
    createImage(swapChainExtent.width, swapChainExtent.height, 1, this->_device.getMsaaSamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _colorImages[0], _colorImageMemorys[0]);
    _colorImageViews[0] = createImageView(_colorImages[0], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void    ft_SwapChain::createSyncObjects()
{
    this->_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    this->_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo   semaphoreInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    VkFenceCreateInfo   fenceInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                        .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(this->_device.device(), &semaphoreInfo, nullptr, &this->_imageAvailableSemaphores[i]) != VK_SUCCESS
            || vkCreateSemaphore(this->_device.device(), &semaphoreInfo, nullptr, &this->_renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(this->_device.device(), &fenceInfo, nullptr, &this->_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
    }
}

VkSurfaceFormatKHR  ft_SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    std::vector<VkSurfaceFormatKHR>::const_iterator it = std::find_if(availableFormats.begin(), availableFormats.end(), [](const VkSurfaceFormatKHR& cur) -> bool {
        if (cur.format == VK_FORMAT_B8G8R8A8_SRGB && cur.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return true;
        return false; 
    });
    return (it == availableFormats.end() ? availableFormats[0] : *it);
}

VkPresentModeKHR    ft_SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D  ft_SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    VkExtent2D actualExtent = this->_windowExtent;
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

VkFormat    ft_SwapChain::findDepthFormat()
{
    return this->_device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}