#ifndef FRAME_HPP
#define FRAME_HPP

#include "Camera.hpp"
#include "GameObject.hpp"

// lib
#include <vulkan/vulkan.h>

struct FrameInfo {
    int                 frameIndex;
    float               frameTime;
    VkCommandBuffer     commandBuffer;
    ft_Camera&             camera;
    VkDescriptorSet     globalDescriptorSet;
    ft_GameObject::Map  &gameObjects;
};

#endif