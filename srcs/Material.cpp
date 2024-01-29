/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Material.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/12 16:47:42 by nflan             #+#    #+#             */
/*   Created: 2023/01/12 16:47:42 by nflan             #+#    #+#             */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Material.hpp"

ft_Material::ft_Material()
{}

ft_Material::ft_Material(std::string p): _file(p)
{
    parseFile();
}

ft_Material::~ft_Material() {}

ft_Material::ft_Material(const ft_Material& o)
{
    *this = o;
}

ft_Material&    ft_Material::operator=(const ft_Material& o)
{
    if (this != &o)
    {
        this->_file = o.getFile();
        this->_materials = o.getMaterials();
    }
    return *this;
}

void    ft_Material::setFile(std::string file)
{
    _file = file;
}

void    ft_Material::parseFile()
{
    std::ifstream in(this->_file, std::ios::in);
    if (!in)
    {
        this->_file.clear();
    }
    long long	i = -1;
    std::vector<Material>	mtl(1);
    std::string	line;
    while (std::getline(in, line))
    {
        if (line.substr(0,7) == "newmtl ")
        {
            i++;
            if (i == mtl.size())
                mtl.resize(mtl.size() + 1);
            std::istringstream(line.substr(7)) >> mtl[i]._newmtl;
        }
        else if (line.substr(0,3) == "Ka ")
            parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._ka);
        else if (line.substr(0,3) == "Kd ")
            parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._kd);
        else if (line.substr(0,3) == "Ks ")
            parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._ks);
        else if (line.substr(0,3) == "Ke ")
            parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._ke);
        else if (line.substr(0, 6) == "illum ")
        {
            std::istringstream	illum(line.substr(6));
            float	il;
            illum >> il;
            if (il == 2)
                mtl[i]._illum = 1;
        }
        else if (line.substr(0,3) == "Ns ")
            std::istringstream(line.substr(3)) >> mtl[i]._ns;
        else if (line.substr(0,3) == "Ni ")
            std::istringstream(line.substr(3)) >> mtl[i]._ni;
        else if (line.substr(0,2) == "d ")
            std::istringstream(line.substr(2)) >> mtl[i]._d;
        else if (line.substr(0, 3) == "Tr ")
            std::istringstream(line.substr(3)) >> mtl[i]._d;
        else if (line.substr(0, 7) == "map_Kd ")
            std::istringstream(line.substr(7)) >> mtl[i]._mapKa;
    }
    for (Material& mat : mtl)
        this->_materials.insert(std::pair<std::string, Material>(mat._newmtl, mat));
    for (std::pair<std::string, Material> print : this->_materials)
        printMaterial(print.second);
    // for (std::map<std::string, Material>::iterator it = _materials.begin(); it != _materials.end(); it++)
    //     printMaterial(it->second);
}

void	ft_Material::parseKaKdKsKe(std::istringstream rgb, std::vector<double>& toFill)
{
	float	R, G , B;
	rgb >> R; rgb >> G; rgb >> B;
	toFill.push_back(R);
	toFill.push_back(G);
	toFill.push_back(B);
}