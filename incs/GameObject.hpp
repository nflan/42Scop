/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameObject.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/02 12:39:39 by nflan             #+#    #+#             */
/*   Updated: 2023/11/02 12:39:39 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include "Model.hpp"

// libs
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

struct TransformComponent {
    glm::vec3   translation{};
    glm::vec3   scale{1.f, 1.f, 1.f};
    glm::vec3   rotation{};

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4();
    glm::mat4 modelMatrix = mat4();
    glm::mat3 normalMatrix();
    glm::mat3 normalMat = normalMatrix();

    void    updateModelMatrix();
    void    updateNormalMatrix();
};

struct PointLightComponent {
    float lightIntensity = 1.f;
};

class ft_GameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, ft_GameObject>;

        static ft_GameObject createGameObject() {
            static id_t currentId = 0;
            return ft_GameObject{currentId++};
        }

        static ft_GameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

        ft_GameObject(const ft_GameObject &) = delete;
        ft_GameObject &operator=(const ft_GameObject &) = delete;
        ft_GameObject(ft_GameObject &&) = default;
        ft_GameObject &operator=(ft_GameObject &&) = default;

        id_t                        getId() { return _id; }

        glm::vec3                   color{};
        TransformComponent          transform{};

        // Optional pointer components
        std::shared_ptr<ft_Model>   model{};
        std::unique_ptr<PointLightComponent>    pointLight = nullptr;

    private:
        ft_GameObject(id_t objId): _id{objId} {}

        id_t    _id;
};

#endif
