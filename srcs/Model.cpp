/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Model.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 19:48:07 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 19:48:08 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Model.hpp"
#include "../incs/tools.hpp"

namespace std
{
    template <>
    struct hash<Vertex> {
        size_t operator()(Vertex const &vertex) const {
            size_t seed = 0;
            std::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

ft_Model::ft_Model(ft_Device &device, std::string mtlFile, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const Material &material) : _device{device}, _material(material), _mtlFile(mtlFile)
{
    createVertexBuffers(vertices);
    createIndexBuffers(indices);
}

ft_Model::~ft_Model() {}

float   ft_Model::calculateBoundingSize()
{
    // Calculate the differences along each axis
    float   deltaX = this->_maxVertice.x - this->_minVertice.x;
    float   deltaY = this->_maxVertice.y - this->_minVertice.y;
    float   deltaZ = this->_maxVertice.z - this->_minVertice.z;

    // Calculate the bounding size (distance between max and min along each axis)
    float boundingSize = glm::sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);

    return boundingSize;
}

std::vector<std::shared_ptr<ft_Model>> ft_Model::createModelFromFile(ft_Device &device, const std::string &filepath)
{
    std::vector<std::shared_ptr<ft_Model>>  Models;

    Loader  load;
    load.LoadFile(filepath);
    for (const Mesh& mesh : load._loadedMeshes) {
        Models.emplace_back(make_unique<ft_Model>(device, load._mtlFile, mesh._vertices, mesh._indices, mesh._meshMaterial));
    }

    glm::vec3   scale = glm::vec3(0.f);
    glm::vec3   tmpCenter = glm::vec3(0.f);
    float       maxBoundarySize = 0.f;

    for (std::shared_ptr<ft_Model>& model : Models) {
        float   boundarySize = model->calculateBoundingSize();
        tmpCenter += model->getCenterOfObj();
        if (maxBoundarySize < boundarySize) {
            scale = model->getScaleObj();
            maxBoundarySize = boundarySize;
        }
    }
    for (std::shared_ptr<ft_Model>& model : Models) {
        model->setCenterOfObj(tmpCenter / glm::vec3(Models.size()));
        model->setScaleObj(scale);
    }
    return Models;
}

void    ft_Model::createVertexBuffers(const std::vector<Vertex> &vertices)
{
    _minVertice = vertices[0].position;
    _maxVertice = vertices[0].position;
    _centerOfObj = glm::vec3(0.f);

    for (const Vertex& vertex : vertices) {
        _minVertice = glm::min(_minVertice, vertex.position);
        _maxVertice = glm::max(_maxVertice, vertex.position);
        _centerOfObj += vertex.position;
    }

    _objectSize = _maxVertice - _minVertice;
    float   maxDimension = glm::max(_objectSize.x, glm::max(_objectSize.y, _objectSize.z));
    _scaleObj = glm::vec3(5.f / maxDimension);//change 5.f to set the scaling

    _centerOfObj /= static_cast<float>(vertices.size());
    this->_vertexCount = static_cast<uint64_t>(vertices.size());

    
    assert(this->_vertexCount >= 3 && "Vertex count must be at least 3");

    VkDeviceSize    bufferSize = sizeof(vertices[0]) * this->_vertexCount;
    uint32_t        vertexSize = sizeof(vertices[0]);

    ft_Buffer   stagingBuffer{
        this->_device,
        vertexSize,
        this->_vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    this->_vertexBuffer = std::make_unique<ft_Buffer>(
        this->_device,
        vertexSize,
        this->_vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    this->_device.copyBuffer(stagingBuffer.getBuffer(), this->_vertexBuffer->getBuffer(), bufferSize);

}

void    ft_Model::createIndexBuffers(const std::vector<uint32_t> &indices)
{
    this->_indexCount = static_cast<uint32_t>(indices.size());
    this->_hasIndexBuffer = this->_indexCount > 0;

    if (!this->_hasIndexBuffer)
        return;

    VkDeviceSize    bufferSize = sizeof(indices[0]) * this->_indexCount;
    uint32_t        indexSize = sizeof(indices[0]);

    ft_Buffer   stagingBuffer{
        this->_device,
        indexSize,
        this->_indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data());

    this->_indexBuffer = std::make_unique<ft_Buffer>(
        this->_device,
        indexSize,
        this->_indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    this->_device.copyBuffer(stagingBuffer.getBuffer(), this->_indexBuffer->getBuffer(), bufferSize);
}

void    ft_Model::draw(VkCommandBuffer commandBuffer)
{
    if (this->_hasIndexBuffer)
        vkCmdDrawIndexed(commandBuffer, this->_indexCount, 1, 0, 0, 0);
    else
        vkCmdDraw(commandBuffer, this->_vertexCount, 1, 0, 0);
}

void    ft_Model::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer        &buffers = this->_vertexBuffer->getBuffer();
    VkDeviceSize    offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers, offsets);

    if (this->_hasIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, this->_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

std::vector<VkVertexInputBindingDescription>     Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription>    bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>   Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription>  attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

    return attributeDescriptions;
}