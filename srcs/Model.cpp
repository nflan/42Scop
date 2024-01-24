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
#include </mnt/nfs/homes/nflan/sgoinfre/bin/tinyobjloader/tiny_obj_loader.h>
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
    _centerOfObj = glm::vec3(0.0f);
    glm::vec3   min(vertices[0].position), max(vertices[0].position);

    for (const Vertex& vertex : vertices)
    {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
        _centerOfObj += vertex.position;
    }
    std::cout << "min.x + min.y + min.z" << " " << min.x << " " << min.y << " " << min.z << std::endl;
    std::cout << "max.x + max.y + max.z" << " " << max.x << " " << max.y << " " << max.z << std::endl;
    std::cout << min.x + min.y + min.z << " " << max.x + max.y + max.z << std::endl;
    std::cout << glm::distance(min, max) << std::endl;
    std::cout << static_cast<float>(8 / glm::distance(min, max)) << std::endl;

    _scaleObj = static_cast<float>(8 / glm::distance(min, max));
    if (_scaleObj < 0)
        _scaleObj *= -1;
    std::cerr << "scale = " << _scaleObj << std::endl;
    // _centerOfObj = _scaleObj * _centerOfObj;
    _centerOfObj /= static_cast<float>(vertices.size());
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

std::vector<VkVertexInputBindingDescription>     ft_Model::Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>   ft_Model::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

    return attributeDescriptions;
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

// void	ft_Model::Builder::loadModel(const std::string &filepath)
// {
// 	tinyobj::attrib_t	attrib;
//     std::vector<tinyobj::shape_t>	shapes;
//     std::vector<tinyobj::material_t>	materials;
//     std::string	warn, err;

//     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
//         throw std::runtime_error(warn + err);

// 	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

// 	for (const auto& shape : shapes)
// 	{
// 		for (const auto& index : shape.mesh.indices)
// 		{
// 			Vertex vertex{};
// 			vertex.position = {
// 				attrib.vertices[3 * index.vertex_index + 0] * -1,
// 				attrib.vertices[3 * index.vertex_index + 1] * -1,
// 				attrib.vertices[3 * index.vertex_index + 2] * -1
// 			};

// 			vertex.uv = {
// 				attrib.texcoords[2 * index.texcoord_index + 0],
// 				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]//textures a l'envers parce que OBJ part d'en bas a gauche et Vulkan en haut a gauche
// 			};

// 			vertex.color = {1.0f, 1.0f, 1.0f};

// 			if (uniqueVertices.count(vertex) == 0)
// 			{
// 				uniqueVertices[vertex] = static_cast<uint32_t>(this->_vertices.size());
// 				this->_vertices.push_back(vertex);
// 			}

// 			this->_indices.push_back(uniqueVertices[vertex]);
// 		}
// 	}
// }

void	ft_Model::Builder::loadModel(const std::string &filepath)
{
    Mesh    mesh;

    mesh.LoadObjModel(filepath);
    std::cout << "mtlfile  = " << mesh.getMtlFile() << std::endl;
	// ft_Material mat(av[1]);
	// tinyobj::attrib_t	attrib;
    // std::vector<tinyobj::shape_t>	shapes;
    // std::vector<tinyobj::material_t>	materials;
    // std::string	warn, err;

    // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
    //     throw std::runtime_error(warn + err);
	this->_vertices.clear();
    this->_indices.clear();
	std::unordered_map<Vertex, uint32_t> uniqueVertices;
    float y = 0;

	for (uint32_t i = 0; i < mesh.getFaceIndex().size(); i++)
	{
		Vertex  vertex{};
        glm::vec3   vertices = mesh.getMeshVertices()[i];

		vertex.position = -vertices; // on peut inverser l'axe dans le viewport mais ca inverse tous les autres calculs et ca trigger les validations layers
        if (mesh.getNormCoord().size() > i)
            vertex.normal = mesh.getNormCoord()[i];
        else
            vertex.normal = -vertices;
		if (mesh.getTexCoord().size() > i)
			vertex.uv = mesh.getTexCoord()[i];
		else
        {
            // std::cerr << "allo" << std::endl;
			vertex.uv = -vertices;
        }
        if (i % 3 == 0)
            y += 0.1;
        if (y >= .6)
            y = 0;
        // std::cout << "y = " << y << " et i = " << i << std::endl;
        vertex.color = glm::vec3(y);
		// vertex.color = {static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX)};

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