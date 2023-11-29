/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Model.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 19:47:42 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 19:47:43 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MODEL_HPP
#define MODEL_HPP

#include "Buffer.hpp"
#include "Device.hpp"
#include "Mesh.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/constants.hpp>

// std
#include <memory>
#include <vector>

class ft_Model {
    public:
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

        struct Builder {
            std::vector<Vertex>     _vertices{};
            std::vector<uint32_t>   _indices{};

            void    loadModel(const std::string &filepath);
        };

        ft_Model(ft_Device &device, const ft_Model::Builder &builder);
        ~ft_Model();

        ft_Model(const ft_Model &) = delete;
        ft_Model &operator=(const ft_Model &) = delete;

        static std::unique_ptr<ft_Model> createModelFromFile(ft_Device &device, const std::string &filepath);

        void    bind(VkCommandBuffer commandBuffer);
        void    draw(VkCommandBuffer commandBuffer);

    private:
        void    createVertexBuffers(const std::vector<Vertex> &vertices);
        void    createIndexBuffers(const std::vector<uint32_t> &indices);

        ft_Device&                  _device;

        std::unique_ptr<ft_Buffer>  _vertexBuffer;
        uint32_t                    _vertexCount;

        bool                        _hasIndexBuffer = false;
        std::unique_ptr<ft_Buffer>  _indexBuffer;
        uint32_t                    _indexCount;
};

#endif