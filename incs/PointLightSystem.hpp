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

// std
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