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

RenderSystem::RenderSystem(ft_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): _device{device}
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
    if (vkCreatePipelineLayout(this->_device.device(), &pipelineLayoutInfo, nullptr, &this->_pipelineLayout) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
}

void    RenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(this->_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    ft_Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    // TODOft_Pipeline::pipelineConfigInfo(pipelineConfig, this->_device);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = this->_pipelineLayout;
    this->_pipeline = std::make_unique<ft_Pipeline>(
        this->_device,
        "shaders/shader.vert.spv",
        "shaders/shader.frag.spv",
        pipelineConfig);
}

void    RenderSystem::renderGameObjects(FrameInfo& frameInfo)
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

    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.model == nullptr) continue;
        SimplePushConstantData push{};
        push.modelMatrix = obj.transform.mat4();
        push.normalMatrix = obj.transform.normalMatrix();

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