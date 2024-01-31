/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 16:10:26 by nflan             #+#    #+#             */
/*   Updated: 2023/10/27 12:46:37 by nflan            ###   ########.fr       */
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
short			WAY = 1;
float			ROTX = 0;
float			ROTY = ROTATION; //change in tools.hpp
float			ROTZ = 0;

static	std::vector<char> readFile(const std::string& filename) {
	std::ifstream	file(filename, std::ios::ate | std::ios::binary); //ate pour commencer a la fin / binary pour dire que c'est un binaire et pas recompiler

	if (!file.is_open())
		throw std::runtime_error(std::string {"Failed to open: "} + filename + "!");
	size_t	fileSize = (size_t) file.tellg(); // on a commence a la fin donc voir ou est le pointeur
	std::vector<char>	buffer(fileSize);

	file.seekg(0); // tout lire jusqu'au debut
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

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
	{
		ROTY = ROTX = ROTZ = 0.f;
	}
	else if ((key == GLFW_KEY_1) && action == GLFW_PRESS)
	{
		ROTY == 0.f ? ROTY = ROTATION : ROTY = 0.f;
	}
	else if ((key == GLFW_KEY_2) && action == GLFW_PRESS)
	{
		ROTX == 0.f ? ROTX = ROTATION : ROTX = 0.f;
	}
	else if ((key == GLFW_KEY_3) && action == GLFW_PRESS)
	{
		ROTZ == 0.f ? ROTZ = ROTATION : ROTZ = 0.f;
	}
	else if ((key == GLFW_KEY_4) && action == GLFW_PRESS)
	{
		ROBJ = true;
	}
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

Display::Display() {
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

Display::Display( const Display & o) {
	if (this != &o)
		*this = o;
	return ;
}

Display &	Display::operator=( const Display& o )
{
	if (this == &o)
		return (*this);
	*this = o;
	return (*this);
}

Display::~Display()
{
	for (Texture text : this->_loadedTextures)
	{
		vkDestroySampler(this->_device.device(), text._sampler, nullptr);
		vkDestroyImageView(this->_device.device(), text._imageView, nullptr);
		vkDestroyImage(this->_device.device(), text._image, nullptr);
		vkFreeMemory(this->_device.device(), text._imageMemory, nullptr);
	}
}

void	Display::setFile(const char* file) { this->_file = file; }
void	Display::setText(const char* file) { this->_textFile = file; }

void	Display::run()
{
	loadGameObjects();
	if (this->_textFile.size())
		loadTextures();

	// addMaterials();
	// for (const std::pair<std::string, ft_Material>& print : this->_materials)
	// 	for (const std::pair<std::string, Material>& p : print.second.getMaterials())
	// 		printMaterial(p.second);
	
	createBuffers();
	createDescriptorSetLayout();
	createDescriptorSets();
	createRenderSystems();
	
	std::cerr << "textfiles: " << std::endl;
	for (std::string s : this->_textFiles)
		std::cerr << s << std::endl;

	ft_Camera camera{};

	ft_GameObject viewerObject = ft_GameObject::createGameObject();

	viewerObject.transform.translation.z = -10.f;
	KeyboardMovementController cameraController(glm::vec3(0,0,-10.f));

	std::chrono::_V2::system_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	while (!this->_window.shouldClose() && !QUIT)
	{
		glfwSetKeyCallback(this->_window.getWindow(), key_callback);
		glfwPollEvents();

		std::chrono::_V2::system_clock::time_point newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ(this->_window.getWindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = this->_renderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

		if (VkCommandBuffer_T *commandBuffer = this->_renderer.beginFrame())
		{
			int	frameIndex = this->_renderer.getFrameIndex();
			FrameInfo frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				RENDER == 1 && ISTEXT ? _descriptorSets[frameIndex] : _descriptorSetsWithoutTexture[frameIndex],
				this->_gameObjects};

			
			GlobalUbo	ubo{};
			ubo.projection = camera.getProjection();
			ubo.view = camera.getView();
			ubo.inverseView = camera.getInverseView();
			this->_pointLightSystems[RENDER]->update(frameInfo, ubo);
			this->_renderSystems[RENDER]->update(frameInfo, ubo);
			this->_buffers[frameIndex]->writeToBuffer(&ubo);
			this->_buffers[frameIndex]->flush();

			// render
			this->_renderer.beginSwapChainRenderPass(commandBuffer);
			
			this->_renderSystems[RENDER]->renderGameObjects(frameInfo);
			this->_pointLightSystems[RENDER]->render(frameInfo);

			this->_renderer.endSwapChainRenderPass(commandBuffer);
			this->_renderer.endFrame();
			if (ISTEXT)
			{
				NBTEXT %= this->_loadedTextures.size();
				if (this->_loadedTextures.size() > 1 && NBTEXT != this->_currText)
				{
					refreshDescriptorSets();
					this->_currText = NBTEXT;
				}
			}
		}
	}

	vkDeviceWaitIdle(this->_device.device());
}

void	Display::loadTextures()
{
	ISTEXT = true;
	getText();

	for (uint32_t i = 0; i < this->_textFiles.size(); i++)
	{
		createTextureImage(this->_textFiles[i].c_str());
		createTextureImageView(this->_loadedTextures[i]);
		createTextureSampler(this->_loadedTextures[i]);
	}
}

void	Display::getText()
{
	if (std::filesystem::exists(this->_textFile))
	{
		if (std::filesystem::is_directory(this->_textFile))
		{
			getTextInDir();
		}
		else if (std::filesystem::is_regular_file(this->_textFile))
			this->_textFiles.push_back(this->_textFile);
	}
	else
	{
		throw std::invalid_argument(std::string("Wrong path: ").append(this->_textFile).c_str());
	}
}

void	Display::getTextInDir()
{
	std::filesystem::directory_entry	dir{std::filesystem::path{this->_textFile}};
	std::vector<std::string>	ext{"png", "jpg", "jpeg"};

	for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator(dir))
	{
		if (entry.is_regular_file())
		{
			bool	add = 0;
			std::string	file(entry.path().filename());
			std::string	extFile(file.c_str(), file.find_last_of('.') + 1, file.size() - (file.find_last_of('.') + 1));
			for (std::string authorizedExt : ext)
				if (authorizedExt == extFile)
					add = !add;
			if (!add)
			{
				std::cerr << "This extension is not usable: " << extFile << ". Try with those one ";
				for (std::string authorizedExt : ext)
				{
					std::cerr << authorizedExt;
					if (authorizedExt != ext[ext.size() - 1])
						std::cerr << ", ";
					else
						std::cerr << ".";
				}
				std::cerr << std::endl; 
			}
			else
				this->_textFiles.push_back(_textFile + "/" + file);
		}
		else
			std::cerr << "Can't use this: " << _textFile << "/" << entry.path().filename() << " for texture." << std::endl;
	}
	for (auto t : _textFiles)
		std::cerr << "t = '" << t << "'" << std::endl;
}

void	Display::createBuffers()
{
	this->_buffers.resize(this->_renderer.getSwapChain().imageCount());
	for (int i = 0; i < this->_renderer.getSwapChain().imageCount(); i++)
	{
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
    std::unique_ptr<ft_DescriptorSetLayout::Builder> builder = std::make_unique<ft_DescriptorSetLayout::Builder>(this->_device);

	builder->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);
    std::unique_ptr<ft_DescriptorSetLayout> colorSetLayout = builder->build();

	this->_globalDescriptorSetLayouts.push_back(std::move(colorSetLayout));
	
	if (this->_loadedTextures.size())
	{
		builder->addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		std::unique_ptr<ft_DescriptorSetLayout> textureSetLayout = builder->build();
		this->_globalDescriptorSetLayouts.push_back(std::move(textureSetLayout));
	}

}

void	Display::createDescriptorSets()
{
	_descriptorSetsWithoutTexture.resize(this->_renderer.getSwapChain().imageCount());
	_descriptorSets.resize(this->_renderer.getSwapChain().imageCount());
	_changeDescriptorSets.resize(this->_renderer.getSwapChain().imageCount());

	for (uint32_t i = 0; i < this->_renderer.getSwapChain().imageCount(); i++)
	{
		std::cerr << "image count " << i << std::endl;
		VkDescriptorBufferInfo bufferInfo = _buffers[i]->descriptorInfo();
		ft_DescriptorWriter(*_globalDescriptorSetLayouts[0], *_globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(_descriptorSetsWithoutTexture[i]);
		if (this->_loadedTextures.size())
		{
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

	for (size_t i = 0; i < this->_renderer.getSwapChain().imageCount(); i++)
	{
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
	std::cerr << this->_globalDescriptorSetLayouts.size() << std::endl;
	for (uint32_t i = 0; i < this->_globalDescriptorSetLayouts.size(); i++)
	{
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
	
// void	Display::addMaterials()
// {
// 	for (std::pair<std::string, ft_Material> mtl : this->_materials)
// 	{
// 		std::cerr << "mtlfile = " << mtl.first << std::endl;
// 		mtl.second.setFile(mtl.first);
// 		mtl.second.parseFile();
// 	}
// 	for (auto& objects : this->_gameObjects)
// 	{
//        	if (objects.second.model == nullptr)
//             continue;
// 		std::map<std::string, ft_Material>::iterator tofind = this->_materials.find(objects.second.model->getMtlFile());
// 		if (tofind != this->_materials.end())
// 			objects.second.model->setMaterial(tofind->second);
// 	}
// }

void	Display::loadGameObjects()
{
  	std::shared_ptr<ft_Model> Model = ft_Model::createModelFromFile(this->_device, this->_file);
	ft_GameObject	gameObj = ft_GameObject::createGameObject();
	gameObj.model = Model;
	// this->_materials.insert(make_pair<std::string, ft_Material>(Model->getMtlFile(), ft_Material()));
	// gameObj.transform.scale = glm::vec3(0.5f);

	// std::cerr << Model->getMaterial().getMaterials().size() << std::endl;
	// std::map<std::string, Material>::iterator end = Model->getMaterial().getMaterials().end();
	// for (std::map<std::string, Material>::iterator it = Model->getMaterial().getMaterials().begin(); it != Model->getMaterial().getMaterials().end(); it++)
	// {
	// 	std::cout << "ptr it = " << *it << std::endl;
	// 	std::cout << "ptr end = " << *end << std::endl;
	// 	std::cout << "Material->first = " << it->first << std::endl;
	// 	printMaterial(it->second);
	// }
	// for (const std::pair<std::string, Material>& material : Model->getMaterial().getMaterials())
    // {
    //     std::cout << "Material.first = " << material.first << std::endl;
    //     printMaterial(material.second);
    // }
	// printMaterial(Model->getMaterial().getMaterials().begin()->second);


	this->_gameObjects.emplace(gameObj.getId(), std::move(gameObj));

	std::vector<glm::vec3> lightColors{
		{1.f, 1.f, 1.f},
		{1.f, 1.f, 1.f},
  	};

	for (int i = 0; i < lightColors.size(); i++)
	{
		ft_GameObject pointLight = ft_GameObject::makePointLight(0.2f);
		pointLight.color = lightColors[i];
		
		glm::mat4 rotateLight = glm::rotate(
			glm::mat4(1.5f),
			(i * glm::two_pi<float>()) / lightColors.size(),
			{0.f, -1.f, 0.f});
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
		this->_gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  	}
}

void	Display::createTextureImage(const char *file)
{
	Texture	text;
	int	texWidth, texHeight, texChannels;
	std::cerr << "file = '" << file << "'" << std::endl;
    stbi_uc	*pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize	imageSize = texWidth * texHeight * 4;

	text._mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
		throw std::runtime_error("failed to load texture image!");

	//Buffer intermediaire image
	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;
	this->_device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void	*data;
	vkMapMemory(this->_device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->_device.device(), stagingBufferMemory);

	stbi_image_free(pixels);

	std::cerr << "msaasamples = " << this->_device.getMsaaSamples() << std::endl;

    this->_renderer.getSwapChain().createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), text._mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, text._image, text._imageMemory);

	this->transitionImageLayout(text._image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, text._mipLevels);
	this->_device.copyBufferToImage(stagingBuffer, text._image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

	vkDestroyBuffer(this->_device.device(), stagingBuffer, nullptr);
    vkFreeMemory(this->_device.device(), stagingBufferMemory, nullptr);

	generateMipmaps(text._image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, text._mipLevels);
	this->_loadedTextures.push_back(text);
}

void	Display::createTextureImageView(Texture &loadedTexture)
{
    loadedTexture._imageView = this->_renderer.getSwapChain().createImageView(loadedTexture._image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, loadedTexture._mipLevels);
}

void	Display::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();

	VkImageMemoryBarrier	barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		throw std::invalid_argument("Unsupported layout transition!");

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
    this->_device.endSingleTimeCommands(commandBuffer);
}

void	Display::createTextureSampler(Texture &loadedTexture)
{

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(this->_device.getPhysicalDevice(), &properties);

	VkSamplerCreateInfo	samplerInfo{}; //permet d'indiquer les filtres et les transformations à appliquer.
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; //interpoler texels magnifies
	samplerInfo.minFilter = VK_FILTER_LINEAR; //interpoler texels minifies

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	/*
	VK_SAMPLER_ADDRESS_MODE_REPEAT : répète le texture
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT : répète en inversant les coordonnées pour réaliser un effet miroir
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : prend la couleur du pixel de bordure le plus proche
	VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : prend la couleur de l'opposé du plus proche côté de l'image
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER : utilise une couleur fixée
	*/
	samplerInfo.anisotropyEnable = VK_TRUE; ///on pourrait le desactiver si le mec n'a pas une cg qui peut le faire
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // et passer ca a 1.f si le mec n'a pas une cg pour le faire
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //si l'image est plus petite que la fenetre, couleur du reste mais que black white or transparent
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	// Le champ unnomalizedCoordinates indique le système de coordonnées que vous voulez utiliser pour accéder aux texels de l'image. Avec VK_TRUE, vous pouvez utiliser des coordonnées dans [0, texWidth) et [0, texHeight). Sinon, les valeurs sont accédées avec des coordonnées dans [0, 1). Dans la plupart des cas les coordonnées sont utilisées normalisées car cela permet d'utiliser un même shader pour des textures de résolution différentes.
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;//Lod -> Level of Details
	samplerInfo.minLod = 0.f; //static_cast<float>(this->_device.getMipLevels() / 2);//minimum de details
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;//static_cast<float>(this->_device.getMipLevels());//maximum de details

	if (vkCreateSampler(this->_device.device(), &samplerInfo, nullptr, &loadedTexture._sampler) != VK_SUCCESS)
        throw std::runtime_error("Fail to create Sampler for texture!");
	// le sampler n'est pas lie a une image ! Il est independant et peut du coup etre efficace tout le long du programme. par contre si on veut changer la facon d'afficher, faut ptete le detruire et le refaire ?
}

void	Display::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	//Verifier si l'image supporte le filtrage lineaire
	VkFormatProperties	formatProperties;
    vkGetPhysicalDeviceFormatProperties(this->_device.getPhysicalDevice(), imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    	throw std::runtime_error("le format de l'image texture ne supporte pas le filtrage lineaire!");

	VkCommandBuffer	commandBuffer = this->_device.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		std::cout << "\ti = " << i << std::endl;
		std::cout << "\tmiplevels = " <<  mipLevels << std::endl;
		std::cout << "\tmipWidth = " <<  mipWidth << std::endl;
		std::cout << "\tmipHeight = " <<  mipHeight << std::endl;
		barrier.subresourceRange.baseMipLevel = i - 1;
		std::cout << "\tbarrier.subresourceRange.baseMipLevel = " <<  barrier.subresourceRange.baseMipLevel << std::endl;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
		
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		std::cout << "\tblit.srcSubresource.mipLevel = " <<  blit.srcSubresource.mipLevel << std::endl;

		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		
		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,//queue graphique obligatoire
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
}

// void	Display::updateUniformBuffer(uint32_t currentImage)
// {
// 	static auto 	startTime = std::chrono::high_resolution_clock::now();

//     auto 	currentTime = std::chrono::high_resolution_clock::now();
//     float 	time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

// 	UniformBufferObject ubo{};
// 	ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f));
// 	ubo.view = glm::lookAt(glm::vec3(5.ff, 5.ff, 5.ff), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
// 	ubo.proj = glm::perspective(glm::radians(45.ff), this->_swapChainExtent.width / (float) this->_swapChainExtent.height, 0.1f, 20.ff);
// 	ubo.proj[1][1] *= -1; //glm fait pour opengl donc inverse x y

// 	void*	data;
// 	vkMapMemory(this->_device.device(), this->_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
//     	memcpy(data, &ubo, sizeof(ubo));
// 	vkUnmapMemory(this->_device.device(), this->_uniformBuffersMemory[currentImage]);
// }