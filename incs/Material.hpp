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

#include <vector>
#include <string>
#include <map>

struct  Material {
    std::string         _newmtl;//name
    std::vector<double>  _ka;//ambient color
    std::vector<double>  _kd;//diffuse color
    std::vector<double>  _ks;//spectacular color
    std::vector<double>  _ke;//couleur émissive (emissive)
    bool                _illum;//illum = 1(0) a flat material with no specular highlights, illum = 2(1) denotes the presence of specular highlights
    double               _ns;//shininess of the material
    double               _ni;//densite optique
    double               _d;//transparency d or Tr
    std::string         _mapKa;//names a file containing a texture map, which should just be an ASCII dump of RGB values
};

class ft_Material
{
    public:
        ft_Material(const char*);
        ~ft_Material() {};
        
        ft_Material(const ft_Material&);
        ft_Material &operator=(const ft_Material&);

        std::map<std::string, Material>&    getMaterial() { return _material; };
        Material&                           getMaterial(std::string s) { return _material.at(s); };
        void                                parseFile();
        void	                            parseKaKdKsKe(std::istringstream, std::vector<double>&);
        void                                printMaterial(Material);
    private:
        ft_Material() {};
        std::string                     _file; 
        std::map<std::string, Material> _material;
};

#endif