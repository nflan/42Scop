/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   KeyboardMovementController.cpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/29 13:02:23 by nflan             #+#    #+#             */
/*   Updated: 2023/11/29 13:02:24 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/KeyboardMovementController.hpp"

#include <limits>

void    KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, ft_GameObject& gameObject)
{
    glm::vec3   rotate{0};
    if (glfwGetKey(window, this->_keys.lookRight) == GLFW_PRESS)
        rotate.y += 1.;
    if (glfwGetKey(window, this->_keys.lookLeft) == GLFW_PRESS)
        rotate.y -= 1.;
    if (glfwGetKey(window, this->_keys.lookUp) == GLFW_PRESS)
        rotate.x += 1.;
    if (glfwGetKey(window, this->_keys.lookDown) == GLFW_PRESS)
        rotate.x -= 1.;
    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        gameObject.transform.rotation += this->_lookSpeed * dt * glm::normalize(rotate);

    // limit pitch values between about +/- 85ish degrees
    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

    float           yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, -1.f, 0.f};

    glm::vec3   moveDir{0.};
    if (glfwGetKey(window, this->_keys.moveForward) == GLFW_PRESS)
        moveDir += forwardDir;
    if (glfwGetKey(window, this->_keys.moveBackward) == GLFW_PRESS)
        moveDir -= forwardDir;
    if (glfwGetKey(window, this->_keys.moveRight) == GLFW_PRESS)
        moveDir += rightDir;
    if (glfwGetKey(window, this->_keys.moveLeft) == GLFW_PRESS)
        moveDir -= rightDir;
    if (glfwGetKey(window, this->_keys.moveUp) == GLFW_PRESS)
        moveDir += upDir;
    if (glfwGetKey(window, this->_keys.moveDown) == GLFW_PRESS)
        moveDir -= upDir;
    if (glfwGetKey(window, this->_keys.reset) == GLFW_PRESS)
        gameObject.transform.translation = _resetPos;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        gameObject.transform.translation += this->_moveSpeed * dt * glm::normalize(moveDir);
}
