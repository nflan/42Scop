/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 12:57:21 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 12:57:22 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Camera.hpp"

// std
#include <cassert>
#include <limits>

void  ft_Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
    this->_projectionMatrix = glm::mat4{1.0f};
    this->_projectionMatrix[0][0] = 2.f / (right - left);
    this->_projectionMatrix[1][1] = 2.f / (bottom - top);
    this->_projectionMatrix[2][2] = 1.f / (far - near);
    this->_projectionMatrix[3][0] = -(right + left) / (right - left);
    this->_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    this->_projectionMatrix[3][2] = -near / (far - near);
}

void  ft_Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFovy = tan(fovy / 2.f);
    this->_projectionMatrix = glm::mat4{0.0f};
    this->_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
    this->_projectionMatrix[1][1] = 1.f / (tanHalfFovy);
    this->_projectionMatrix[2][2] = far / (far - near);
    this->_projectionMatrix[2][3] = 1.f;
    this->_projectionMatrix[3][2] = -(far * near) / (far - near);
}

void  ft_Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    const glm::vec3 w{glm::normalize(direction)};
    const glm::vec3 u{glm::normalize(glm::cross(w, up))};
    const glm::vec3 v{glm::cross(w, u)};

    this->_viewMatrix = glm::mat4{1.f};
    this->_viewMatrix[0][0] = u.x;
    this->_viewMatrix[1][0] = u.y;
    this->_viewMatrix[2][0] = u.z;
    this->_viewMatrix[0][1] = v.x;
    this->_viewMatrix[1][1] = v.y;
    this->_viewMatrix[2][1] = v.z;
    this->_viewMatrix[0][2] = w.x;
    this->_viewMatrix[1][2] = w.y;
    this->_viewMatrix[2][2] = w.z;
    this->_viewMatrix[3][0] = -glm::dot(u, position);
    this->_viewMatrix[3][1] = -glm::dot(v, position);
    this->_viewMatrix[3][2] = -glm::dot(w, position);
}

void  ft_Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    setViewDirection(position, target - position, up);
}

void  ft_Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
    this->_viewMatrix = glm::mat4{1.f};
    this->_viewMatrix[0][0] = u.x;
    this->_viewMatrix[1][0] = u.y;
    this->_viewMatrix[2][0] = u.z;
    this->_viewMatrix[0][1] = v.x;
    this->_viewMatrix[1][1] = v.y;
    this->_viewMatrix[2][1] = v.z;
    this->_viewMatrix[0][2] = w.x;
    this->_viewMatrix[1][2] = w.y;
    this->_viewMatrix[2][2] = w.z;
    this->_viewMatrix[3][0] = -glm::dot(u, position);
    this->_viewMatrix[3][1] = -glm::dot(v, position);
    this->_viewMatrix[3][2] = -glm::dot(w, position);
}
