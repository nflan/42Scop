/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 12:55:57 by nflan             #+#    #+#             */
/*   Updated: 2023/10/31 12:55:58 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CAMERA_HPP
#define CAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>

class ft_Camera {
    public:
        void    setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
        void    setPerspectiveProjection(float fovy, float aspect, float near, float far);

        void    setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
        void    setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
        void    setViewYXZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4&    getProjection() const { return _projectionMatrix; }
        const glm::mat4&    getView() const { return _viewMatrix; }
        const glm::mat4&    getInverseView() const { return _inverseViewMatrix; }
        const glm::vec3 getPosition() const { return glm::vec3(_inverseViewMatrix[3]); }

    private:
        glm::mat4   _projectionMatrix{1.f};
        glm::mat4   _viewMatrix{1.f};
        glm::mat4   _inverseViewMatrix{1.f};
};

#endif