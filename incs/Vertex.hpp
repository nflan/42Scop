/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Vertex.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/07 17:31:59 by nflan             #+#    #+#             */
/*   Updated: 2024/02/07 17:32:00 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VERTEX_HPP
#define VERTEX_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3   position{};
    glm::vec3   color{};
    glm::vec3   normal{};
    glm::vec2   uv{};

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
    return position == other.position && color == other.color && normal == other.normal &&
            uv == other.uv;
    }
};

#endif
