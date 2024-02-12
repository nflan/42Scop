/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PointLightSystem.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/02 11:56:53 by nflan             #+#    #+#             */
/*   Updated: 2023/11/02 11:56:53 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POINTLIGHTSYSTEM_HPP
#define POINTLIGHTSYSTEM_HPP

#include "Device.hpp"
#include "FrameInfo.hpp"
#include "Pipeline.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/constants.hpp>

#ifndef DEBUG
#define NDEBUG
#endif
// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>
#include <memory>
#include <vector>

class PointLightSystem {
    public:
        PointLightSystem(ft_Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void    update(FrameInfo &frameInfo, GlobalUbo &ubo);
        void    render(FrameInfo &frameInfo);

    private:
        void    createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void    createPipeline(VkRenderPass renderPass);

        ft_Device&                      _device;

        std::unique_ptr<ft_Pipeline>    _pipeline;
        VkPipelineLayout                _pipelineLayout;
};

#endif