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
#include "Material.hpp"
#include "Loader.hpp"
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
        struct Builder {
            std::vector<Vertex>     _vertices{};
            std::vector<uint32_t>   _indices{};
            std::string             _file;
            std::string             _path;
            std::string             _mtlFile;

            void    loadModel(const std::string &filepath);
        };

        ft_Model(ft_Device &device, const ft_Model::Builder &builder);
        ~ft_Model();

        ft_Model(const ft_Model &) = delete;
        ft_Model &operator=(const ft_Model &) = delete;

        static std::unique_ptr<ft_Model>    createModelFromFile(ft_Device &device, const std::string &filepath);
        glm::vec3                           getCenterOfObj( void ) { return _centerOfObj; };
        double                              getScaleObj( void ) { return _scaleObj; };
        std::string                         getMtlFile() { return _mtlFile; };
        ft_Material&                        getMaterial() { return _material; };


        void    bind(VkCommandBuffer commandBuffer);
        void    draw(VkCommandBuffer commandBuffer);

    private:
        void    createVertexBuffers(const std::vector<Vertex> &vertices);
        void    createIndexBuffers(const std::vector<uint32_t> &indices);
        void    createMaterial(const std::string& mtlFile);

        ft_Device&                  _device;

        std::unique_ptr<ft_Buffer>  _vertexBuffer;
        uint32_t                    _vertexCount;

        bool                        _hasIndexBuffer = false;
        std::unique_ptr<ft_Buffer>  _indexBuffer;
        uint32_t                    _indexCount;
        glm::vec3                   _centerOfObj;
        double                      _scaleObj;
        std::string                 _mtlFile;
        ft_Material                 _material;
};

#endif