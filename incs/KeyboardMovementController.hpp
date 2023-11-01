/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   KeyboardMovementController.hpp                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:37:41 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 14:37:42 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef KEYBOARDMOVEMENTCONTROLLER_HPP
#define KEYBOARDMOVEMENTCONTROLLER_HPP

#include "GameObject.hpp"
#include "Window.hpp"

class   KeyboardMovementController {

    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;

        };

        void    moveInPlaneXZ(GLFWwindow* window, float dt, ft_GameObject& gameObject);

        KeyMappings _keys{};
        float       _moveSpeed{3.f};
        float       _lookSpeed{1.5f};
};

#endif