/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pipeline.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 19:30:47 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 19:30:48 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Pipeline.hpp"

ft_Pipeline::ft_Pipeline(ft_Device& device,const std::string& vertFilepath,const std::string& fragFilepath,const PipelineConfigInfo& configInfo): _device{device}
{
    createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
}

ft_Pipeline::~ft_Pipeline()
{
    vkDestroyShaderModule(this->_device.device(), this->_vertShaderModule, nullptr);
    vkDestroyShaderModule(this->_device.device(), this->_fragShaderModule, nullptr);
    vkDestroyPipeline(this->_device.device(), this->_graphicsPipeline, nullptr);
}

std::vector<char>   ft_Pipeline::readFile(const std::string& filename)
{
	std::ifstream	file(filename, std::ios::ate | std::ios::binary); //ate pour commencer a la fin / binary pour dire que c'est un binaire et pas recompiler

	if (!file.is_open())
		throw std::runtime_error(std::string {"Fail to open file: "} + filename + "!");
	size_t	fileSize = (size_t) file.tellg();
	std::vector<char>	buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void ft_Pipeline::createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo)
{
    assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
    assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in configInfo");

    auto vertShaderCode = readFile(vertFilepath);
    auto fragShaderCode = readFile(fragFilepath);

    createShaderModule(vertShaderCode, &this->_vertShaderModule);
    createShaderModule(fragShaderCode, &this->_fragShaderModule);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                        .pNext = nullptr,
                                                        .flags = 0,
                                                        .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                                        .module = this->_vertShaderModule,
                                                        .pName = "main",
                                                        .pSpecializationInfo = nullptr}; //to optimize

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                        .pNext = nullptr,
                                                        .flags = 0,
                                                        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                        .module = this->_fragShaderModule,
                                                        .pName = "main",
                                                        .pSpecializationInfo = nullptr};

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto& bindingDescriptions = configInfo.bindingDescriptions;
    auto& attributeDescriptions = configInfo.attributeDescriptions;

    VkPipelineVertexInputStateCreateInfo    vertexInputInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                                                            .vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
                                                            .pVertexBindingDescriptions = bindingDescriptions.data(),
                                                            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
                                                            .pVertexAttributeDescriptions = attributeDescriptions.data()};

    VkGraphicsPipelineCreateInfo    pipelineInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                    .stageCount = 2,
                                                    .pStages = shaderStages,
                                                    .pVertexInputState = &vertexInputInfo,
                                                    .pInputAssemblyState = &configInfo.inputAssemblyInfo,
                                                    .pViewportState = &configInfo.viewportInfo,
                                                    .pRasterizationState = &configInfo.rasterizationInfo,
                                                    .pMultisampleState = &configInfo.multisampleInfo,
                                                    .pDepthStencilState = &configInfo.depthStencilInfo,
                                                    .pColorBlendState = &configInfo.colorBlendInfo,
                                                    .pDynamicState = &configInfo.dynamicStateInfo,
                                                    .layout = configInfo.pipelineLayout,
                                                    .renderPass = configInfo.renderPass,
                                                    .subpass = configInfo.subpass,
                                                    .basePipelineHandle = VK_NULL_HANDLE,
                                                    .basePipelineIndex = -1};

    if (vkCreateGraphicsPipelines(this->_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->_graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline");
}

void ft_Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo    createInfo{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                            .codeSize = code.size(),
                                            .pCode = reinterpret_cast<const uint32_t*>(code.data())};

	if (vkCreateShaderModule(this->_device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module");
}

void    ft_Pipeline::bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_graphicsPipeline);
}

void ft_Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
    //interessant pour points / lignes / triangles / contigus ou non : https://vulkan-tutorial.com/fr/Dessiner_un_triangle/Pipeline_graphique_basique/Fonctions_fixees
    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = nullptr;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = nullptr;

	//RASTERIZER
    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.lineWidth = 1.f;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.f;  // Optional
    configInfo.rasterizationInfo.depthBiasClamp = 0.f;           // Optional
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.f;     // Optional

    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 0.2;           // Optional
    configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

    configInfo.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.f;  // Optional
    configInfo.colorBlendInfo.blendConstants[1] = 0.f;  // Optional
    configInfo.colorBlendInfo.blendConstants[2] = 0.f;  // Optional
    configInfo.colorBlendInfo.blendConstants[3] = 0.f;  // Optional

    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.f;  // Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.f;  // Optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {};  // Optional
    configInfo.depthStencilInfo.back = {};   // Optional

    configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
    configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    configInfo.dynamicStateInfo.flags = 0;

    configInfo.bindingDescriptions = Vertex::getBindingDescriptions();
    configInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
}

void    ft_Pipeline::enableAlphaBlending(PipelineConfigInfo& configInfo)
{
    configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
    configInfo.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}