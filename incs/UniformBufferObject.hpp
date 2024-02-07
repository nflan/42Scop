/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UniformBufferObject.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/07 17:32:06 by nflan             #+#    #+#             */
/*   Updated: 2024/02/07 17:32:06 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UNIFORMBUFFEROBJECT_HPP
#define UNIFORMBUFFEROBJECT_HPP

#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>


struct UniformBufferObject {
    alignas(16) glm::mat4   model;
    alignas(16) glm::mat4   view;
    alignas(16) glm::mat4   proj;
};

#endif
