/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameObject.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/02 12:39:44 by nflan             #+#    #+#             */
/*   Updated: 2023/11/02 12:40:11 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/GameObject.hpp"

glm::mat4 TransformComponent::mat4()
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	return glm::mat4
	{
		{
			scale.x * (c1 * c3 + s1 * s2 * s3),
			scale.x * (c2 * s3),
			scale.x * (c1 * s2 * s3 - c3 * s1),
			0.0f,
		},
		{
			scale.y * (c3 * s1 * s2 - c1 * s3),
			scale.y * (c2 * c3),
			scale.y * (c1 * c3 * s2 + s1 * s3),
			0.0f,
		},
		{
			scale.z * (c2 * s1),
			scale.z * (-s2),
			scale.z * (c1 * c2),
			0.0f,
		},
		// 		{
		// 	scale.x * (c1 * c3 - s1 * c2 * s3),
		// 	scale.x * (s1 * c3 + c1 * c2 *s3),
		// 	scale.x * (s2 * s3),
		// 	0.0f,
		// },
		// {
		// 	scale.y * ((-c1) * s3 - s1 * c2 * c3),
		// 	scale.y * ((-s1) * s3 + c1 *c2 *c3),
		// 	scale.y * (s2 * c3),
		// 	0.0f,
		// },
		// {
		// 	scale.z * (s1 * s2),
		// 	scale.z * ((-c1) * s2),
		// 	scale.z * (c2),
		// 	0.0f,
		// },
		{translation.x, translation.y, translation.z, 1.0f}};
}

glm::mat3 TransformComponent::normalMatrix() {
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 invScale = 1.0f / scale;

	return glm::mat3
	{
		{
			invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
		},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			},
	};
}

void	TransformComponent::updateModelMatrix()
{
	modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, translation);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, scale);
}

void	TransformComponent::updateNormalMatrix()
{
	normalMat = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
}

ft_GameObject   ft_GameObject::makePointLight(float intensity, float radius, glm::vec3 color)
{
	ft_GameObject gameObj = ft_GameObject::createGameObject();
	gameObj.color = color;
	gameObj.transform.scale.x = radius;
	gameObj.pointLight = std::make_unique<PointLightComponent>();
	gameObj.pointLight->lightIntensity = intensity;
	return gameObj;
}
