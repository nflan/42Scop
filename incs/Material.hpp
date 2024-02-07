/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Material.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/12 16:47:42 by nflan             #+#    #+#             */
/*   Created: 2023/01/12 16:47:42 by nflan             #+#    #+#             */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#define GLFW_INCLUDE_VULKAN
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>
#include <map>
#include <utility>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "tools.hpp"

class ft_Material
{
    public:
        ft_Material();
        ft_Material(std::string);
        ~ft_Material();
        
        ft_Material(const ft_Material&);
        ft_Material &operator=(const ft_Material&);

        const std::map<std::string, Material>&  getMaterials() const { return _materials; };
        const Material&                         getMaterial(std::string s) const { return _materials.at(s); };
        const std::string&                      getFile() const { return _file; };
        void                                    setFile(std::string);  
        void                                    parseFile();
        void	                                parseKaKdKsKe(std::istringstream, glm::vec3&);
    private:
        std::string                     _file; 
        std::map<std::string, Material> _materials;
};

#endif