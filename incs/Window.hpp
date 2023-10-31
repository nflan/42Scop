/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:54:18 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 14:54:19 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WINDOW_HPP
#define WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class ft_Window {
    public:
        ft_Window(int w, int h, std::string name);
        ~ft_Window();

        ft_Window(const ft_Window &) = delete;
        ft_Window &operator=(const ft_Window &) = delete;

        GLFWwindow* getWindow() { return _window; }
        
        bool        shouldClose() { return glfwWindowShouldClose(_window); }
        VkExtent2D  getExtent() { return {static_cast<uint32_t>(_width), static_cast<uint32_t>(_height)}; }
        bool        wasWindowResize() { return _framebufferResized; }
        void        resetWindowResizedFlag() { _framebufferResized = false; }

        void        createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
        void        initWindow();

        int         _width;
        int         _height;
        bool        _framebufferResized = false;

        std::string _windowName;
        GLFWwindow* _window;
};

#endif
