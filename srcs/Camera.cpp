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

void    ft_Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
    _projectionMatrix = glm::mat4{1.f};
    _projectionMatrix[0][0] = 2.f / (right - left);
    _projectionMatrix[1][1] = 2.f / (bottom - top);
    _projectionMatrix[2][2] = 1.f / (far - near);
    _projectionMatrix[3][0] = -(right + left) / (right - left);
    _projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    _projectionMatrix[3][2] = -near / (far - near);
}

void    ft_Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.f);
    
    const float tanHalfFovy = tan(fovy / 2.f);
    _projectionMatrix = glm::mat4{0.f};
    _projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
    _projectionMatrix[1][1] = 1.f / (tanHalfFovy);
    _projectionMatrix[2][2] = far / (far - near);
    _projectionMatrix[2][3] = 1.f;
    _projectionMatrix[3][2] = -(far * near) / (far - near);
}

void    ft_Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    const glm::vec3 w{glm::normalize(direction)};
    const glm::vec3 u{glm::normalize(glm::cross(w, up))};
    const glm::vec3 v{glm::cross(w, u)};

    _viewMatrix = glm::mat4{1.f};
    _viewMatrix[0][0] = u.x;
    _viewMatrix[1][0] = u.y;
    _viewMatrix[2][0] = u.z;
    _viewMatrix[0][1] = v.x;
    _viewMatrix[1][1] = v.y;
    _viewMatrix[2][1] = v.z;
    _viewMatrix[0][2] = w.x;
    _viewMatrix[1][2] = w.y;
    _viewMatrix[2][2] = w.z;
    _viewMatrix[3][0] = -glm::dot(u, position);
    _viewMatrix[3][1] = -glm::dot(v, position);
    _viewMatrix[3][2] = -glm::dot(w, position);

    _inverseViewMatrix = glm::mat4{1.f};
    _inverseViewMatrix[0][0] = u.x;
    _inverseViewMatrix[0][1] = u.y;
    _inverseViewMatrix[0][2] = u.z;
    _inverseViewMatrix[1][0] = v.x;
    _inverseViewMatrix[1][1] = v.y;
    _inverseViewMatrix[1][2] = v.z;
    _inverseViewMatrix[2][0] = w.x;
    _inverseViewMatrix[2][1] = w.y;
    _inverseViewMatrix[2][2] = w.z;
    _inverseViewMatrix[3][0] = position.x;
    _inverseViewMatrix[3][1] = position.y;
    _inverseViewMatrix[3][2] = position.z;
}

void    ft_Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    setViewDirection(position, target - position, up);
}

void    ft_Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
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
    _viewMatrix = glm::mat4{1.f};
    _viewMatrix[0][0] = u.x;
    _viewMatrix[1][0] = u.y;
    _viewMatrix[2][0] = u.z;
    _viewMatrix[0][1] = v.x;
    _viewMatrix[1][1] = v.y;
    _viewMatrix[2][1] = v.z;
    _viewMatrix[0][2] = w.x;
    _viewMatrix[1][2] = w.y;
    _viewMatrix[2][2] = w.z;
    _viewMatrix[3][0] = -glm::dot(u, position);
    _viewMatrix[3][1] = -glm::dot(v, position);
    _viewMatrix[3][2] = -glm::dot(w, position);

    _inverseViewMatrix = glm::mat4{1.f};
    _inverseViewMatrix[0][0] = u.x;
    _inverseViewMatrix[0][1] = u.y;
    _inverseViewMatrix[0][2] = u.z;
    _inverseViewMatrix[1][0] = v.x;
    _inverseViewMatrix[1][1] = v.y;
    _inverseViewMatrix[1][2] = v.z;
    _inverseViewMatrix[2][0] = w.x;
    _inverseViewMatrix[2][1] = w.y;
    _inverseViewMatrix[2][2] = w.z;
    _inverseViewMatrix[3][0] = position.x;
    _inverseViewMatrix[3][1] = position.y;
    _inverseViewMatrix[3][2] = position.z;
}