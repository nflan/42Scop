/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RenderSystem.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 20:05:28 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 20:05:28 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RENDERSYSTEM_HPP
#define RENDERSYSTEM_HPP

#include "Camera.hpp"
#include "Device.hpp"
// #include "lve_frame_info.hpp"
// #include "lve_game_object.hpp"
#include "Pipeline.hpp"

#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>


// std
#include <memory>
#include <vector>

class RenderSystem {
    public:
        RenderSystem(ft_Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        // void renderGameObjects(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        ft_Device&                      _device;

        std::unique_ptr<ft_Pipeline>    _pipeline;
        VkPipelineLayout                _pipelineLayout;
};

#endif