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
#include "FrameInfo.hpp"
#include "tools.hpp"
#include "GameObject.hpp"
#include "Pipeline.hpp"

#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/quaternion.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtx/transform.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>

// std
#include <memory>

class RenderSystem {
    public:
        RenderSystem(ft_Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, std::string shader);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void    update(FrameInfo &frameInfo, GlobalUbo &ubo);
        void    update(ft_GameObject& model, GlobalUbo& ubo);
        void    renderGameObjects(FrameInfo &frameInfo);
        void    renderGameObjects(FrameInfo& frameInfo, std::unique_ptr<ft_Buffer>* buffer, GlobalUbo& ubo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        ft_Device&                      _device;
        std::string                     _shader;

        std::unique_ptr<ft_Pipeline>    _pipeline;
        VkPipelineLayout                _pipelineLayout;
};

#endif