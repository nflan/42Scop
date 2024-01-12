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

#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iomanip>

ft_Material::ft_Material(const char* file): _file(file)
{
    parseFile();
}

void    ft_Material::parseFile()
{
    std::ifstream in(this->_file, std::ios::in);
    if (!in)
    {
        std::cerr << "Cannot open " << this->_file << std::endl;
        exit(1);
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
    for (Material mat : mtl)
        this->_material.insert(std::pair<std::string, Material>(mat._newmtl, mat));
    for (std::pair<std::string, Material> print : this->_material)
        printMaterial(print.second);
}

void	ft_Material::parseKaKdKsKe(std::istringstream rgb, std::vector<double>& toFill)
{
	float	R, G , B;
	rgb >> R; rgb >> G; rgb >> B;
	toFill.push_back(R);
	toFill.push_back(G);
	toFill.push_back(B);
}

void	ft_Material::printMaterial(Material print)
{
    std::cout << std::fixed << std::setprecision(7);
	std::cout << std::endl << "--------- Material ---------" << std::endl;
	std::cout << "Newmtl = '" << print._newmtl << "'" << std::endl;
	if (print._ka.size())
	std::cout << "Ka = '" << print._ka[0] << "' '" << print._ka[1] << "' '" << print._ka[2] << "'" << std::endl;
	if (print._kd.size())
	std::cout << "Kd = '" << print._kd[0] << "' '" << print._kd[1] << "' '" << print._kd[2] << "'" << std::endl;
	if (print._ks.size())
	std::cout << "Ks = '" << print._ks[0] << "' '" << print._ks[1] << "' '" << print._ks[2] << "'" << std::endl;
	if (print._ke.size())
		std::cout << "Ke = '" << print._ke[0] << "' '" << print._ke[1] << "' '" << print._ke[2] << "'" << std::endl;
	std::cout << "illum = '" << print._illum << "'" << std::endl;
	std::cout << "Ns = '" << print._ns << "'" << std::endl;
	std::cout << "Ni = '" << print._ni << "'" << std::endl;
	std::cout << "d = '" << print._d << "'" << std::endl;
	if (print._mapKa.size())
	std::cout << "mapKa = '" << print._mapKa << "'" << std::endl;
}

// void	parseMtlFile(const std::string &filename)
// {
// 	std::cout << std::fixed << std::setprecision(7);
//     std::ifstream in(filename, std::ios::in);
//     if (!in)
//     {
//         std::cerr << "Cannot open " << filename << std::endl;
//         exit(1);
//     }
// 	long long	i = -1;
// 	std::vector<Material>	mtl(1);
// 	std::string	line;
// 	while (std::getline(in, line))
//     {
//         if (line.substr(0,7) == "newmtl ")
//         {
// 			i++;
// 			if (i == mtl.size())
// 				mtl.resize(mtl.size() + 1);
//             std::istringstream(line.substr(7)) >> mtl[i]._newmtl;
//         }
//         else if (line.substr(0,3) == "Ka ")
// 			parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._ka);
// 		else if (line.substr(0,3) == "Kd ")
// 			parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._kd);
// 		else if (line.substr(0,3) == "Ks ")
// 			parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._ks);
// 		else if (line.substr(0,3) == "Ke ")
// 			parseKaKdKsKe(std::istringstream(line.substr(3)), mtl[i]._ke);
// 		else if (line.substr(0, 6) == "illum ")
// 		{
// 			std::istringstream	illum(line.substr(6));
// 			float	il;
// 			illum >> il;
// 			if (il == 2)
// 				mtl[i]._illum = 1;
// 		}
// 		else if (line.substr(0,3) == "Ns ")
// 			std::istringstream(line.substr(3)) >> mtl[i]._ns;
// 		else if (line.substr(0,3) == "Ni ")
// 			std::istringstream(line.substr(3)) >> mtl[i]._ni;
// 		else if (line.substr(0,2) == "d ")
// 			std::istringstream(line.substr(2)) >> mtl[i]._d;
// 		else if (line.substr(0, 3) == "Tr ")
// 			std::istringstream(line.substr(3)) >> mtl[i]._d;
// 		else if (line.substr(0, 7) == "map_Kd ")
// 			std::istringstream(line.substr(7)) >> mtl[i]._mapKa;
// 	}
// 	for (Material print : mtl)
// 		printMaterial(print);
// }