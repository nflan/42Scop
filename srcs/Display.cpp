/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:10:26 by nflan             #+#    #+#             */
/*   Updated: 2023/10/27 12:46:38 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Display.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "/mnt/nfs/homes/nflan/sgoinfre/bin/stb/stb_image.h"

bool			QUIT = false;
bool			ROBJ = false;
bool			ISTEXT = false;
int				RENDER = 0;
unsigned int	NBTEXT = 0;
short			WAY = -1;
float			ROTX = 0;
float			ROTY = ROTATION; //change in tools.hpp
float			ROTZ = 0;
glm::mat4   	ROTATE = glm::mat4(
        1, 0, 0, 0,
        0, -1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1
        );

void	key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static_cast<void> (window);
	static_cast<void> (scancode);
	static_cast<void> (mods);
	if ((key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
		QUIT = true;
	else if ((key == GLFW_KEY_F) && action == GLFW_PRESS)
		WAY *= -1;
	else if ((key == GLFW_KEY_P) && action == GLFW_PRESS)
		ROTY = ROTX = ROTZ = 0.f;
	else if ((key == GLFW_KEY_1) && action == GLFW_PRESS)
		ROTY == 0.f ? ROTY = ROTATION : ROTY = 0.f;
	else if ((key == GLFW_KEY_2) && action == GLFW_PRESS)
		ROTX == 0.f ? ROTX = ROTATION : ROTX = 0.f;
	else if ((key == GLFW_KEY_3) && action == GLFW_PRESS)
		ROTZ == 0.f ? ROTZ = ROTATION : ROTZ = 0.f;
	else if ((key == GLFW_KEY_4) && action == GLFW_PRESS)
		ROBJ = true;
	else if ((key == GLFW_KEY_C) && action == GLFW_PRESS)
	{
		if (ISTEXT)
			RENDER == 0 ? RENDER = 1 : RENDER = 0;
	}
	else if ((key == GLFW_KEY_T) && action == GLFW_PRESS)
	{
		if (ISTEXT)
			NBTEXT++;
	}
	
}

Display::Display()
{
	_currText = 0;
	_globalPool =
		ft_DescriptorPool::Builder(_device)
		.setMaxSets(this->_renderer.getSwapChain().imageCount())
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->_renderer.getSwapChain().imageCount())
		.build();
	_globalPoolText =
		ft_DescriptorPool::Builder(_device)
		.setMaxSets(this->_renderer.getSwapChain().imageCount())
		.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->_renderer.getSwapChain().imageCount())
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->_renderer.getSwapChain().imageCount())
		.build();
	_changePoolText =
		ft_DescriptorPool::Builder(_device)
		.setMaxSets(this->_renderer.getSwapChain().imageCount())
		.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->_renderer.getSwapChain().imageCount())
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->_renderer.getSwapChain().imageCount())
		.build();
}

Display::Display( const Display & o)
{
	if (this != &o)
		*this = o;
	return ;
}

Display	&Display::operator=( const Display& o )
{
	if (this == &o)
		return (*this);
	*this = o;
	return (*this);
}

Display::~Display()
{
	for (Texture& text : this->_loadedTextures)
	{
		vkDestroySampler(this->_device.device(), text._sampler, nullptr);
		vkDestroyImageView(this->_device.device(), text._imageView, nullptr);
		vkDestroyImage(this->_device.device(), text._image, nullptr);
		vkFreeMemory(this->_device.device(), text._imageMemory, nullptr);
	}
}

void	Display::setFile(const char* file) { this->_file = file; }
void	Display::setText(const char* file) { this->_textFile = file; }

bool	Display::run()
{
	loadGameObjects();
	if (this->_textFile.size())
	{
		if (loadTextures())
			return (1);
	}
	
	createBuffers();
	createDescriptorSetLayout();
	createDescriptorSets();
	createRenderSystems();

	ft_Camera									camera{};
	ft_GameObject								viewerObject = ft_GameObject::createGameObject();
	KeyboardMovementController					cameraController(glm::vec3(0,0,-10.f));
	std::chrono::_V2::system_clock::time_point	currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::_V2::system_clock::time_point	newTime;

	viewerObject.transform.translation.z = -10.f;
	
	while (!this->_window.shouldClose() && !QUIT)
	{
		glfwSetKeyCallback(this->_window.getWindow(), key_callback);
		glfwPollEvents();

		newTime = std::chrono::high_resolution_clock::now();
		float	frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ(this->_window.getWindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = this->_renderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

		if (VkCommandBuffer_T *commandBuffer = this->_renderer.beginFrame())
		{
			int	frameIndex = this->_renderer.getFrameIndex();

			FrameInfo	frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				RENDER == 1 && ISTEXT ? _descriptorSets[frameIndex] : _descriptorSetsWithoutTexture[frameIndex],
				this->_gameObjects};
			
			GlobalUbo	ubo{.projection = camera.getProjection(),
							.view = camera.getView(),
							.inverseView = camera.getInverseView()};
			this->_pointLightSystems[RENDER]->update(frameInfo, ubo);

			// render
			this->_renderer.beginSwapChainRenderPass(commandBuffer);
			
			this->_renderSystems[RENDER]->renderGameObjects(frameInfo, &this->_buffers[frameIndex], ubo);
			this->_pointLightSystems[RENDER]->render(frameInfo);

			this->_renderer.endSwapChainRenderPass(commandBuffer);
			this->_renderer.endFrame();
			if (ISTEXT)
			{
				if (NBTEXT >= this->_loadedTextures.size())
					NBTEXT = 0;
				if (this->_loadedTextures.size() > 1 && NBTEXT != this->_currText)
				{
					refreshDescriptorSets();
					this->_currText = NBTEXT;
				}
			}
		}
	}

	vkDeviceWaitIdle(this->_device.device());
	return (0);
}

bool	Display::loadTextures()
{
	if (getText())
		return (0);
	if (this->_textFiles.size())
	{
		ISTEXT = true;	
		for (uint32_t i = 0; i < this->_textFiles.size(); i++)
		{
			if (createTextureImage(this->_textFiles[i].c_str()))
				return (1);
			createTextureImageView(this->_loadedTextures[i]);
			createTextureSampler(this->_loadedTextures[i]);
		}
	}
	return (0);
}

bool	Display::getText()
{
	if (std::filesystem::exists(this->_textFile))
	{
		if (std::filesystem::is_directory(this->_textFile)) {
			getTextInDir();
		}
		else if (std::filesystem::is_regular_file(this->_textFile) && !isTexFile(this->_textFile) && !testOpenFile(this->_textFile)) {
			this->_textFiles.push_back(this->_textFile);
		}
		else
			return (1);
	} else {
		throw std::invalid_argument("Wrong path: " + this->_textFile);
	}
	return (0);
}

void	Display::getTextInDir()
{
	std::filesystem::directory_entry	dir{std::filesystem::path{this->_textFile}};
	std::vector<std::string>			ext{"png", "jpg", "jpeg"};

	for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator(dir))
	{
		if (entry.is_regular_file())
		{
			std::string	file = this->_textFile;
			if (*(file.end() - 1) != '/')
				file += "/";
			file += entry.path().filename();
			if (isTexFile(file)) {
				continue ;
			}
			else if (!testOpenFile(file)) {
				this->_textFiles.push_back(file);
			}
		} else {
			std::cerr << "Can't use this: '" << _textFile << "/" << entry.path().filename() << "' for texture." << std::endl;
		}
	}
	#ifdef DEBUG
	{
		for (std::string t : _textFiles)
			std::cerr << "t = '" << t << "'" << std::endl;
	}
	#endif
}

void	Display::createBuffers()
{
	this->_buffers.resize(this->_renderer.getSwapChain().imageCount());
	for (size_t i = 0; i < this->_renderer.getSwapChain().imageCount(); i++) {
		std::unique_ptr<ft_Buffer> buffer = std::make_unique<ft_Buffer>(
			this->_device,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        buffer->map();
        this->_buffers[i] = std::move(buffer);
	}
}

void	Display::createDescriptorSetLayout()
{
    std::unique_ptr<ft_DescriptorSetLayout::Builder>	builder = std::make_unique<ft_DescriptorSetLayout::Builder>(this->_device);

	builder->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);
    std::unique_ptr<ft_DescriptorSetLayout>	colorSetLayout = builder->build();

	this->_globalDescriptorSetLayouts.push_back(std::move(colorSetLayout));
	
	if (this->_loadedTextures.size()) {
		builder->addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		std::unique_ptr<ft_DescriptorSetLayout>	textureSetLayout = builder->build();
		this->_globalDescriptorSetLayouts.push_back(std::move(textureSetLayout));
	}

}

void	Display::createDescriptorSets()
{
	_descriptorSetsWithoutTexture.resize(this->_renderer.getSwapChain().imageCount());
	_descriptorSets.resize(this->_renderer.getSwapChain().imageCount());
	_changeDescriptorSets.resize(this->_renderer.getSwapChain().imageCount());

	for (uint64_t i = 0; i < this->_renderer.getSwapChain().imageCount(); i++) {
		VkDescriptorBufferInfo bufferInfo = _buffers[i]->descriptorInfo();
		ft_DescriptorWriter(*_globalDescriptorSetLayouts[0], *_globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(_descriptorSetsWithoutTexture[i]);
		if (this->_loadedTextures.size()) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = this->_loadedTextures[0]._imageView;
			imageInfo.sampler = this->_loadedTextures[0]._sampler;
			ft_DescriptorWriter(*_globalDescriptorSetLayouts[1], *_globalPoolText)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.build(_descriptorSets[i]);
		}
	}
}

void	Display::refreshDescriptorSets()
{
	VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();
	this->_device.endSingleTimeCommands(commandBuffer);

	for (uint64_t i = 0; i < this->_renderer.getSwapChain().imageCount(); i++) {
		VkDescriptorBufferInfo bufferInfo = _buffers[i]->descriptorInfo();
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = this->_loadedTextures[NBTEXT]._imageView;
		imageInfo.sampler = this->_loadedTextures[NBTEXT]._sampler;

		ft_DescriptorWriter(*_globalDescriptorSetLayouts[1], *_changePoolText)
			.writeBuffer(0, &bufferInfo)
			.writeImage(1, &imageInfo)
			.build(_changeDescriptorSets[i]);
	}
	_descriptorSets.swap(_changeDescriptorSets);
	_globalPoolText.swap(_changePoolText);
	vkFreeDescriptorSets( this->_device.device(), this->_changePoolText.get()->getDescriptorPool(), static_cast<uint32_t>(this->_changeDescriptorSets.size()), this->_changeDescriptorSets.data() );
}

void	Display::createRenderSystems()
{
	for (uint64_t i = 0; i < this->_globalDescriptorSetLayouts.size(); i++) {
		_renderSystems.emplace_back(std::make_unique<RenderSystem>(
			this->_device,
			this->_renderer.getSwapChainRenderPass(),
			this->_globalDescriptorSetLayouts[i]->getDescriptorSetLayout(),
			i == 0 ? "color_shader" : "texture_shader"
		));

		_pointLightSystems.emplace_back(std::make_unique<PointLightSystem>(
			this->_device,
			this->_renderer.getSwapChainRenderPass(),
			this->_globalDescriptorSetLayouts[i]->getDescriptorSetLayout()
		));
	}
}

void	Display::loadGameObjects()
{
  	std::vector<std::shared_ptr<ft_Model>>	Model = ft_Model::createModelFromFile(this->_device, this->_file);
	for (size_t i = 0; i < Model.size(); i++) {
		ft_GameObject	gameObj = ft_GameObject::createGameObject();
		gameObj.model = Model[i];
		this->_gameObjects.emplace(gameObj.getId(), std::move(gameObj));
	}

	std::vector<glm::vec3> lightColors{
		{1.f, 1.f, 1.f}
  	};//add light color if needed

	for (int i = 0; i < MAX_LIGHTS; i++) {
		ft_GameObject	pointLight = ft_GameObject::makePointLight(.03f);
		pointLight.color = lightColors[i];
		
		glm::mat4	rotateLight = glm::rotate(
			glm::mat4(1.5f),
			(i * glm::two_pi<float>()) / lightColors.size(),
			{0.f, -1.f, 0.f});
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -2.f, 1.f));
		this->_gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  	}
}

bool	Display::createTextureImage(const char *file)
{
	Texture			text;
	int				texWidth, texHeight, texChannels;
    stbi_uc			*pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize	imageSize = texWidth * texHeight * 4;

	text._mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
	{
		std::cerr << "Failed to load thexture image from file '" << file << "'!" << std::endl;
		return (1);
	}
	//Buffer intermediaire image
	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;
	try {
		this->_device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	}
	catch (const std::exception& e) {
		stbi_image_free(pixels);
		throw std::runtime_error(e.what());
	}

	void	*data;
	vkMapMemory(this->_device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->_device.device(), stagingBufferMemory);

	stbi_image_free(pixels);

    this->_renderer.getSwapChain().createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), text._mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, text._image, text._imageMemory);
	if (this->transitionImageLayout(text._image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, text._mipLevels))
	{
		vkDestroyBuffer(this->_device.device(), stagingBuffer, nullptr);
		vkFreeMemory(this->_device.device(), stagingBufferMemory, nullptr);
		vkDestroyImage(this->_device.device(), text._image, nullptr);
		vkFreeMemory(this->_device.device(), text._imageMemory, nullptr);
		return (1);
	}
	this->_device.copyBufferToImage(stagingBuffer, text._image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

	vkDestroyBuffer(this->_device.device(), stagingBuffer, nullptr);
	vkFreeMemory(this->_device.device(), stagingBufferMemory, nullptr);

	if (generateMipmaps(text._image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, text._mipLevels)) {
		vkDestroyImage(this->_device.device(), text._image, nullptr);
		vkFreeMemory(this->_device.device(), text._imageMemory, nullptr);
		return (1);
	}
	this->_loadedTextures.push_back(text);
	return (0);
}

void	Display::createTextureImageView(Texture &loadedTexture)
{
    loadedTexture._imageView = this->_renderer.getSwapChain().createImageView(loadedTexture._image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, loadedTexture._mipLevels);
}

bool	Display::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();

	VkImageMemoryBarrier	barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
									.srcAccessMask = 0,
									.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
									.oldLayout = oldLayout,
									.newLayout = newLayout,
									.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
									.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
									.image = image,
									.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
														.baseMipLevel = 0,
														.levelCount = mipLevels,
														.baseArrayLayer = 0,
														.layerCount = 1}};

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout != VK_IMAGE_LAYOUT_UNDEFINED && newLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		return (error("Unsupported layout transition!"));
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
    this->_device.endSingleTimeCommands(commandBuffer);
	return (0);
}

void	Display::createTextureSampler(Texture &loadedTexture)
{

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(this->_device.getPhysicalDevice(), &properties);

	VkSamplerCreateInfo	samplerInfo{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,//permet d'indiquer les filtres et les transformations à appliquer.
									.magFilter = VK_FILTER_LINEAR,//interpoler texels magnifies
									.minFilter = VK_FILTER_LINEAR,//interpoler texels minifies
									.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
									.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
									.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
									.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
									.mipLodBias = 0.f,//Lod -> Level of Details
									.anisotropyEnable = VK_TRUE,//we can desable it if the user's gc don't manage
									.maxAnisotropy = properties.limits.maxSamplerAnisotropy,//and use 1.f
									.compareEnable = VK_FALSE,
									.compareOp = VK_COMPARE_OP_ALWAYS,
									.minLod = 0.f,// static_cast<float>(loadedTexture._mipLevels / 2);//minimum de details (rend flou de pres)
									.maxLod = VK_LOD_CLAMP_NONE,//static_cast<float>(loadedTexture._mipLevels);//maximum de details
									.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,//si l'image est plus petite que la fenetre, couleur du reste mais que black white or transparent
									.unnormalizedCoordinates = VK_FALSE};// Le champ unnomalizedCoordinates indique le système de coordonnées que vous voulez utiliser pour accéder aux texels de l'image. Avec VK_TRUE, vous pouvez utiliser des coordonnées dans [0, texWidth) et [0, texHeight). Sinon, les valeurs sont accédées avec des coordonnées dans [0, 1). Dans la plupart des cas les coordonnées sont utilisées normalisées car cela permet d'utiliser un même shader pour des textures de résolution différentes.

	/*
	VK_SAMPLER_ADDRESS_MODE_REPEAT : répète la texture
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT : répète en inversant les coordonnées pour réaliser un effet miroir
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : prend la couleur du pixel de bordure le plus proche
	VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : prend la couleur de l'opposé du plus proche côté de l'image
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER : utilise une couleur fixée
	*/

	if (vkCreateSampler(this->_device.device(), &samplerInfo, nullptr, &loadedTexture._sampler) != VK_SUCCESS)
        throw std::runtime_error("Fail to create Sampler for texture!");
	// le sampler n'est pas lie a une image ! Il est independant et peut du coup etre efficace tout le long du programme. par contre si on veut changer la facon d'afficher, faut ptete le detruire et le refaire ?
}

bool	Display::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	//Verifier si l'image supporte le filtrage lineaire
	VkFormatProperties	formatProperties;
    vkGetPhysicalDeviceFormatProperties(this->_device.getPhysicalDevice(), imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    	return (error("Image texture format does not accept linear filter!"));

	VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
									.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
									.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
									.image = image,
									.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
														.levelCount = 1,
														.baseArrayLayer = 0,
														.layerCount = 1}
								};

	int32_t	mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);


		VkImageBlit blit{.srcSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
											.mipLevel = i - 1,
											.baseArrayLayer = 0,
											.layerCount = 1},
							.srcOffsets = {{0, 0, 0}, {mipWidth, mipHeight, 1}},
							.dstSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
											.mipLevel = i,
											.baseArrayLayer = 0,
											.layerCount = 1},
							.dstOffsets = {{0, 0, 0}, {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}}
		};

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,//Graphic Queue mandatory
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

    this->_device.endSingleTimeCommands(commandBuffer);
	return (0);
}
