/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 15:06:05 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 15:06:06 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Window.hpp"

#include <stdexcept>

ft_Window::ft_Window(int w, int h, std::string name) : _width{w}, _height{h}, _windowName{name} {
  initWindow();
}

ft_Window::~ft_Window() {
  glfwDestroyWindow(this->_window);
  glfwTerminate();
}

void ft_Window::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  this->_window = glfwCreateWindow(this->_width, this->_height, this->_windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(this->_window, this);
  glfwSetFramebufferSizeCallback(this->_window, framebufferResizeCallback);
}

void ft_Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, this->_window, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to craete window surface");
  }
}

void ft_Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  auto Window = reinterpret_cast<ft_Window *>(glfwGetWindowUserPointer(window));
  Window->_framebufferResized = true;
  Window->_width = width;
  Window->_height = height;
}