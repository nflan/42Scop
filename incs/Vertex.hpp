/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Vertex.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/26 16:32:50 by nflan             #+#    #+#             */
/*   Updated: 2023/10/27 12:18:12 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VERTEX_HPP
#define VERTEX_HPP

#define GLFW_INCLUDE_VULKAN
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <array>

struct	Vertex {
	glm::vec3	pos;
	glm::vec3	color;
	glm::vec2	texCoord;

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }

	static VkVertexInputBindingDescription	getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		//VK_VERTEX_INPUT_RATE_VERTEX : Passer au jeu de données suivante après chaque sommet
		//VK_VERTEX_INPUT_RATE_INSTANCE : Passer au jeu de données suivantes après chaque instance: Instanced rendering ?

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3>	attributeDescriptions{};
		//vertex
		attributeDescriptions[0].binding = 0;//index -> bindingdescription
		attributeDescriptions[0].location = 0;//location fichier shader
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		/*
float : VK_FORMAT_R32_SFLOAT
vec2 : VK_FORMAT_R32G32_SFLOAT
vec3 : VK_FORMAT_R32G32B32_SFLOAT
vec4 : VK_FORMAT_R32G32B32A32_SFLOAT
types : SFLOAT / UINT / SINT
ivec2 correspond à VK_FORMAT_R32G32_SINT et est un vecteur à deux composantes d'entiers signés de 32 bits
uvec4 correspond à VK_FORMAT_R32G32B32A32_UINT et est un vecteur à quatre composantes d'entiers non signés de 32 bits
double correspond à VK_FORMAT_R64_SFLOAT et est un float à précision double (donc de 64 bits)
*/
		attributeDescriptions[0].offset = offsetof(Vertex, pos);//calcul automatique de decalage dans les donnes extraites pour avoir le debut de la variable avec offsetof

		//couleur
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

#endif
