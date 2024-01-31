/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FrameInfo.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/02 12:25:43 by nflan             #+#    #+#             */
/*   Updated: 2023/11/02 12:25:44 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FRAMEINFO_HPP
#define FRAMEINFO_HPP

#include "Camera.hpp"
#include "GameObject.hpp"
#include "tools.hpp"

// lib
#include <vulkan/vulkan.h>

#define MAX_LIGHTS 2

struct PointLight {
    glm::vec4   position{};  // ignore w
    glm::vec4   color{};     // w is intensity
};

struct GlobalUbo {
    alignas(16) glm::mat4   projection{1.f};
    alignas(16) glm::mat4   view{1.f};
    alignas(16) glm::mat4   inverseView{1.f};
    alignas(16) glm::vec3   ka{1.f, 1.f, 1.f}; // ambient color
    alignas(16) glm::vec3   kd{0.f, 0.f, 0.f}; // diffuse color
    alignas(16) glm::vec3   ks{0.f, 0.f, 0.f}; // specular color
    alignas(16) glm::vec3   ke{0.f, 0.f, 0.f}; // emissive color
    alignas(16) PointLight  pointLights[MAX_LIGHTS];
    alignas(4) float       ni{.08f}; //intensity
    alignas(4) float       ns{512.f}; //
    alignas(4) float       d{1.f};
    alignas(4) int         illum{1};
    alignas(4) int         numLights;
};

struct FrameInfo {
    int                 frameIndex;
    float               frameTime;
    VkCommandBuffer     commandBuffer;
    ft_Camera&          camera;
    VkDescriptorSet     globalDescriptorSet;
    ft_GameObject::Map& gameObjects;
};

#endif
