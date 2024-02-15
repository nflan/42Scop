/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RenderSystem.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 20:05:34 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 20:05:34 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/RenderSystem.hpp"

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

RenderSystem::RenderSystem(ft_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string shader): _device{device}, _shader{shader}
{
    createPipelineLayout(globalSetLayout);
    try {
        createPipeline(renderPass);
    }
    catch (const std::exception& e) {
        vkDestroyPipelineLayout(this->_device.device(), this->_pipelineLayout, nullptr);
        throw std::runtime_error(e.what());
    }
}

RenderSystem::~RenderSystem()
{
    vkDestroyPipelineLayout(this->_device.device(), this->_pipelineLayout, nullptr);
}

void    RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                            .offset = 0,
                                            .size = sizeof(SimplePushConstantData)};

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                    .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
                                                    .pSetLayouts = descriptorSetLayouts.data(),
                                                    .pushConstantRangeCount = 1,
                                                    .pPushConstantRanges = &pushConstantRange};

    if (vkCreatePipelineLayout(this->_device.device(), &pipelineLayoutInfo, nullptr, &this->_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
}

void    RenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(this->_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    ft_Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.multisampleInfo.rasterizationSamples = this->_device.getMsaaSamples();
    pipelineConfig.pipelineLayout = this->_pipelineLayout;
    this->_pipeline = std::make_unique<ft_Pipeline>(
        this->_device,
        "shaders/" + _shader + ".vert.spv",
        "shaders/" + _shader + ".frag.spv",
        pipelineConfig);
}

void    RenderSystem::update(ft_GameObject& model, GlobalUbo& ubo)
{
    #ifdef DEBUG
        std::cerr << "UBO before : " << std::endl;
        std::cerr << "- ka = " << ubo.ka.x << "," << ubo.ka.y << "," << ubo.ka.z << std::endl;
        std::cerr << "- kd = " << ubo.kd.x << "," << ubo.kd.y << "," << ubo.kd.z << std::endl;
        std::cerr << "- ks = " << ubo.ks.x << "," << ubo.ks.y << "," << ubo.ks.z << std::endl;
        std::cerr << "- ke = " << ubo.ke.x << "," << ubo.ke.y << "," << ubo.ke.z << std::endl;
        std::cerr << "- illum = " << ubo.illum << std::endl;
        std::cerr << "- ns = " << ubo.ns << std::endl;
        std::cerr << "- ni = " << ubo.ni << std::endl;
        std::cerr << "- d = " << ubo.d << std::endl;
	#endif
    if (model.model->getMtlFile().size())
    {
        #ifdef DEBUG
            std::cerr << "mtlfile = " << model.model->getMtlFile() << std::endl;
        #endif
        Material    light = model.model->getMaterial();
        ubo.ka = light._ka;
        ubo.kd = light._kd;
        ubo.ks = light._ks;
        ubo.ke = light._ke;
        ubo.illum = light._illum;
        ubo.ni = light._ni;
        ubo.ns = light._ns;
        ubo.d = light._d;
    }
    #ifdef DEBUG
        std::cerr << "UBO after : " << std::endl;
        std::cerr << "- ka = " << ubo.ka.x << "," << ubo.ka.y << "," << ubo.ka.z << std::endl;
        std::cerr << "- kd = " << ubo.kd.x << "," << ubo.kd.y << "," << ubo.kd.z << std::endl;
        std::cerr << "- ks = " << ubo.ks.x << "," << ubo.ks.y << "," << ubo.ks.z << std::endl;
        std::cerr << "- ke = " << ubo.ke.x << "," << ubo.ke.y << "," << ubo.ke.z << std::endl;
        std::cerr << "- illum = " << ubo.illum << std::endl;
        std::cerr << "- ns = " << ubo.ns << std::endl;
        std::cerr << "- ni = " << ubo.ni << std::endl;
        std::cerr << "- d = " << ubo.d << std::endl;
        std::cerr << "Size of GlobalUbo: " << sizeof(GlobalUbo) << std::endl;
        std::cerr << "Alignment of GlobalUbo: " << alignof(GlobalUbo) << std::endl;
	#endif
}

void    RenderSystem::renderGameObjects(FrameInfo& frameInfo, std::unique_ptr<ft_Buffer>* buffer, GlobalUbo& ubo)
{
    this->_pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        this->_pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    for (std::pair<const ft_GameObject::id_t, ft_GameObject>& kv : frameInfo.gameObjects) {
        ft_GameObject   &obj = kv.second;
        if (obj.model == nullptr)
            continue;
        update(obj, ubo);
        buffer->get()->writeToBuffer(&ubo);
        buffer->get()->flush();

        obj.transform.scale = glm::vec3(obj.model->getScaleObj());
        obj.transform.translation = -obj.model->getCenterOfObj();

        obj.transform.rotation += glm::vec3(ROTX * WAY, ROTY * WAY, ROTZ * WAY); // rotate on itself
        if (ROBJ) {
            obj.transform.rotation = glm::vec3(0.f, 0.f, 0.f);
            ROBJ = false;
        }
        glm::quat   rotationQuat = glm::quat(obj.transform.rotation); // quaternions est plus interessant que euler pour ce calcul. Eulers > humanoid / camera

        obj.transform.modelMatrix =
            glm::mat4_cast(rotationQuat) *
            glm::scale(glm::mat4(1.f), obj.transform.scale) *
            glm::translate(glm::mat4(1.f), obj.transform.translation);

        obj.transform.modelMatrix = ROTATE * obj.transform.modelMatrix;
        obj.transform.updateNormalMatrix();
        SimplePushConstantData  push{.modelMatrix = obj.transform.modelMatrix,
                                        .normalMatrix = obj.transform.normalMat};
        vkCmdPushConstants(
            frameInfo.commandBuffer,
            this->_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);
        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
    }
}