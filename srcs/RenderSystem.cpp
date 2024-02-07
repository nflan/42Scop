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

// std
#include <array>
#include <cassert>
#include <stdexcept>

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

RenderSystem::RenderSystem(ft_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string shader): _device{device}, _shader{shader}
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

RenderSystem::~RenderSystem()
{
    vkDestroyPipelineLayout(this->_device.device(), this->_pipelineLayout, nullptr);
}

void    RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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
    // std::cerr << "UBO before : " << std::endl;
    // std::cerr << "- ka = " << ubo.ka.x << "," << ubo.ka.y << "," << ubo.ka.z << std::endl;
    // std::cerr << "- kd = " << ubo.kd.x << "," << ubo.kd.y << "," << ubo.kd.z << std::endl;
    // std::cerr << "- ks = " << ubo.ks.x << "," << ubo.ks.y << "," << ubo.ks.z << std::endl;
    // std::cerr << "- ke = " << ubo.ke.x << "," << ubo.ke.y << "," << ubo.ke.z << std::endl;
    // std::cerr << "- illum = " << ubo.illum << std::endl;
    // std::cerr << "- ns = " << ubo.ns << std::endl;
    // std::cerr << "- ni = " << ubo.ni << std::endl;
    // std::cerr << "- d = " << ubo.d << std::endl;
    if (model.model->getMtlFile().size())
    {
        // std::cout << "mtlfile = " << model.model->getMtlFile() << std::endl;
        Material    light = model.model->getMaterial();
        // std::cerr << "MTL NAME = " << light._name << std::endl;
        ubo.ka = light._ka;
        ubo.kd = light._kd;
        ubo.ks = light._ks;
        ubo.ke = light._ke;
        ubo.illum = light._illum;
        ubo.ni = light._ni;
        ubo.ns = light._ns;
        ubo.d = light._d;
    }
    // std::cerr << "UBO after : " << std::endl;
    // std::cerr << "- ka = " << ubo.ka.x << "," << ubo.ka.y << "," << ubo.ka.z << std::endl;
    // std::cerr << "- kd = " << ubo.kd.x << "," << ubo.kd.y << "," << ubo.kd.z << std::endl;
    // std::cerr << "- ks = " << ubo.ks.x << "," << ubo.ks.y << "," << ubo.ks.z << std::endl;
    // std::cerr << "- ke = " << ubo.ke.x << "," << ubo.ke.y << "," << ubo.ke.z << std::endl;
    // std::cerr << "- illum = " << ubo.illum << std::endl;
    // std::cerr << "- ns = " << ubo.ns << std::endl;
    // std::cerr << "- ni = " << ubo.ni << std::endl;
    // std::cerr << "- d = " << ubo.d << std::endl;
    // std::cerr << "Size of GlobalUbo: " << sizeof(GlobalUbo) << std::endl;
    // std::cerr << "Alignment of GlobalUbo: " << alignof(GlobalUbo) << std::endl;
}

       #include <unistd.h>
       #include <strings.h>


void    RenderSystem::renderGameObjects(FrameInfo& frameInfo, std::unique_ptr<ft_Buffer>* buffer, GlobalUbo& ubo)
{
    this->_pipeline->bind(frameInfo.commandBuffer);
    int i = 0;

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        this->_pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    for (std::pair<const ft_GameObject::id_t, ft_GameObject>& kv : frameInfo.gameObjects)
    {
        ft_GameObject   &obj = kv.second;
        if (obj.model == nullptr)
            continue;
        i++;
        update(obj, ubo);
        // bzero(buffer->get()->getBuffer(), buffer->get()->getBufferSize());
        buffer->get()->writeToBuffer(&ubo);
        buffer->get()->flush();

        obj.transform.scale = glm::vec3(obj.model->getScaleObj());
        obj.transform.translation = -obj.model->getCenterOfObj();

        obj.transform.rotation += glm::vec3(ROTX * WAY, ROTY * WAY, ROTZ * WAY); // rotate on itself
        if (ROBJ)
        {
            obj.transform.rotation = glm::vec3(0.f, 0.f, 0.f);
            ROBJ = false;
        }
        glm::quat   rotationQuat = glm::quat(obj.transform.rotation); // quaternions est plus interessant que euler pour ce calcul. Eulers > humanoid / camera

        obj.transform.modelMatrix =
            glm::mat4_cast(rotationQuat) *
            glm::scale(glm::mat4(1.f), obj.transform.scale) *
            glm::translate(glm::mat4(1.f), obj.transform.translation);

        SimplePushConstantData  push{};
        obj.transform.modelMatrix = ROTATE * obj.transform.modelMatrix;
        push.modelMatrix = obj.transform.modelMatrix;
        obj.transform.updateNormalMatrix();
        push.normalMatrix = obj.transform.normalMat;
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