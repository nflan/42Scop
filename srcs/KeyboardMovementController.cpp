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

void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, ft_GameObject& gameObject)
{
    glm::vec3   rotate{0};
    if (glfwGetKey(window, this->_keys.lookRight) == GLFW_PRESS) rotate.y += 1.;
    if (glfwGetKey(window, this->_keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.;
    if (glfwGetKey(window, this->_keys.lookUp) == GLFW_PRESS) rotate.x += 1.;
    if (glfwGetKey(window, this->_keys.lookDown) == GLFW_PRESS) rotate.x -= 1.;
    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.rotation += this->_lookSpeed * dt * glm::normalize(rotate);
    }

    // limit pitch values between about +/- 85ish degrees
    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

    float   yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0., cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0., -forwardDir.x};
    const glm::vec3 upDir{0., -1., 0.};

    glm::vec3   moveDir{0.};
    if (glfwGetKey(window, this->_keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, this->_keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, this->_keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, this->_keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, this->_keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, this->_keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
    if (glfwGetKey(window, this->_keys.reset) == GLFW_PRESS)
    {
        gameObject.transform.translation = _resetPos;//+ glm::normalize(moveDir);
    }

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        gameObject.transform.translation += this->_moveSpeed * dt * glm::normalize(moveDir);
}
