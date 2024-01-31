/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PointLightSystem.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/02 11:56:58 by nflan             #+#    #+#             */
/*   Updated: 2023/11/02 11:56:59 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/PointLightSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

struct PointLightPushConstants {
    glm::vec4   position{};
    glm::vec4   color{};
    float       radius;
};

PointLightSystem::PointLightSystem(ft_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): _device{device}
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem()
{
  vkDestroyPipelineLayout(this->_device.device(), this->_pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PointLightPushConstants);

    std::vector<VkDescriptorSetLayout>  descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    if (vkCreatePipelineLayout(this->_device.device(), &pipelineLayoutInfo, nullptr, &this->_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
}

void    PointLightSystem::createPipeline(VkRenderPass renderPass)
{
    assert(this->_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    ft_Pipeline::defaultPipelineConfigInfo(pipelineConfig);//, this->_device);
    ft_Pipeline::enableAlphaBlending(pipelineConfig);
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.multisampleInfo.rasterizationSamples = this->_device.getMsaaSamples();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = this->_pipelineLayout;
    _pipeline = std::make_unique<ft_Pipeline>(
        this->_device,
        "shaders/point_light.vert.spv",
        "shaders/point_light.frag.spv",
        pipelineConfig);
}

void    PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
{
    auto    rotateLight = glm::rotate(glm::mat4(1.f), .5f * frameInfo.frameTime, {0.f, -1.f, 0.f});
    int lightIndex = 0;
    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.pointLight == nullptr)
            continue;

        assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

        // update light position
        obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.0));

        // copy light to ubo
        ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
        ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

        lightIndex += 1;
    }
    ubo.numLights = lightIndex;
}

void    PointLightSystem::render(FrameInfo& frameInfo)
{
    // sort lights
    std::map<float, ft_GameObject::id_t> sorted;
    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.pointLight == nullptr)
            continue;

        // calculate distance
        auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
        float disSquared = glm::dot(offset, offset);
        sorted[disSquared] = obj.getId();
    }

    _pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        this->_pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    // iterate through sorted lights in reverse order
    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
        // use game obj id to find light object
        auto& obj = frameInfo.gameObjects.at(it->second);

        PointLightPushConstants push{};
        push.position = glm::vec4(obj.transform.translation, 1.f);
        push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
        push.radius = obj.transform.scale.x;

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            this->_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PointLightPushConstants),
            &push);
        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
}