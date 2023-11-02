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

// libs
#define TINYOBJLOADER_IMPLEMENTATION
// #include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std {
    template <>
    struct hash<ft_Model::Vertex> {
        size_t operator()(ft_Model::Vertex const &vertex) const {
            size_t seed = 0;
            std::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

ft_Model::ft_Model(ft_Device &device, const ft_Model::Builder &builder) : _device{device}
{
    createVertexBuffers(builder._vertices);
    createIndexBuffers(builder._indices);
}

ft_Model::~ft_Model() {}

std::unique_ptr<ft_Model> ft_Model::createModelFromFile(
    ft_Device &device, const std::string &filepath)
{
    Builder builder{};
    builder.loadModel(filepath);
    return std::make_unique<ft_Model>(device, builder);
}

void    ft_Model::createVertexBuffers(const std::vector<Vertex> &vertices)
{
    this->_vertexCount = static_cast<uint32_t>(vertices.size());
    assert(this->_vertexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize    bufferSize = sizeof(vertices[0]) * this->_vertexCount;
    uint32_t    vertexSize = sizeof(vertices[0]);

    ft_Buffer stagingBuffer{
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
    {
        return;
    }

    VkDeviceSize    bufferSize = sizeof(indices[0]) * this->_indexCount;
    uint32_t    indexSize = sizeof(indices[0]);

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
    {
        vkCmdDrawIndexed(commandBuffer, this->_indexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(commandBuffer, this->_vertexCount, 1, 0, 0);
    }
}

void    ft_Model::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {this->_vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (this->_hasIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, this->_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

/*void    ft_Model::Builder::loadModel(const std::string &filepath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
        Vertex vertex{};

        if (index.vertex_index >= 0) {
            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };

            vertex.color = {
                attrib.colors[3 * index.vertex_index + 0],
                attrib.colors[3 * index.vertex_index + 1],
                attrib.colors[3 * index.vertex_index + 2],
            };
        }

        if (index.normal_index >= 0) {
            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
            };
        }

        if (index.texcoord_index >= 0) {
            vertex.uv = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1],
            };
        }

        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
        }
    }
}*/

void	ft_Model::Builder::loadModel(const std::string &filepath)
{
    Mesh    mesh;

    mesh.LoadObjModel(filepath);
	// tinyobj::attrib_t	attrib;
    // std::vector<tinyobj::shape_t>	shapes;
    // std::vector<tinyobj::material_t>	materials;
    // std::string	warn, err;

    // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
    //     throw std::runtime_error(warn + err);
	this->_vertices.clear();
	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (uint32_t i = 0; i < mesh.getFaceIndex().size(); i++)
	{
		Vertex vertex{};

		vertex.position = mesh.getMeshVertices()[i];
		if (mesh.getTexCoord().size() > i)
			vertex.uv = mesh.getTexCoord()[i];
		else
			vertex.uv = {0.0f, 0.0f};
		vertex.color = {static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX)};

		if (uniqueVertices.count(vertex) == 0)
		{
			uniqueVertices[vertex] = static_cast<uint32_t>(this->_vertices.size());
			this->_vertices.push_back(vertex);
		}
		// this->_vertices.push_back(vertex);
		// this->_indices.push_back(this->_indices.size());
		this->_indices.push_back(uniqueVertices[vertex]);
	}
}