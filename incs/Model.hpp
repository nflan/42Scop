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
#include "Loader.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtx/hash.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/constants.hpp>

// std
#ifndef DEBUG
#define NDEBUG
#endif
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <memory>
#include <vector>

class ft_Model {
    public:
        struct Builder {
            std::vector<Vertex>     _vertices{};
            std::vector<uint32_t>   _indices{};
            std::string             _mtlFile;
            std::string             _path;
            std::string             _file;

            void                    loadModel(const std::string &filepath);
        };

        ft_Model(ft_Device &device, std::string mtlFile, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const Material &materials);
        ~ft_Model();

        ft_Model(const ft_Model &) = delete;
        ft_Model    &operator=(const ft_Model &) = delete;

        static std::vector<std::shared_ptr<ft_Model>>   createModelFromFile(ft_Device &device, const std::string &filepath);
        glm::vec3                                       getCenterOfObj( void ) { return _centerOfObj; };
        glm::vec3                                       getScaleObj( void ) { return _scaleObj; };
        glm::vec3                                       getMaxVertice( void ) { return _maxVertice; };
        glm::vec3                                       getMinVertice( void ) { return _minVertice; };
        glm::vec3                                       getObjectSize( void ) { return _objectSize; };
        std::string                                     getMtlFile() { return _mtlFile; };
        const Material&                                 getMaterial() { return _material; };
        void                                            setCenterOfObj(glm::vec3 c) { _centerOfObj = c; }
        void                                            setScaleObj(glm::vec3 s) { _scaleObj = s; }

        float                                           calculateBoundingSize();
        void                                            bind(VkCommandBuffer commandBuffer);
        void                                            draw(VkCommandBuffer commandBuffer);

    private:
        void                        createVertexBuffers(const std::vector<Vertex> &vertices);
        void                        createIndexBuffers(const std::vector<uint32_t> &indices);
        ft_Device&                  _device;

        std::unique_ptr<ft_Buffer>  _vertexBuffer;
        uint32_t                    _vertexCount;

        bool                        _hasIndexBuffer = false;
        std::unique_ptr<ft_Buffer>  _indexBuffer;
        uint32_t                    _indexCount;
        glm::vec3                   _centerOfObj;
        glm::vec3                   _scaleObj;
        glm::vec3                   _maxVertice;
        glm::vec3                   _minVertice;
        glm::vec3                   _objectSize;
        std::string                 _mtlFile;
        const Material              _material;
};

#endif