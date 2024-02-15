/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/25 15:39:44 by nflan             #+#    #+#             */
/*   Updated: 2024/01/25 12:40:11 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/tools.hpp"

void	printMaterial(Material print)
{
    std::cout << std::fixed << std::setprecision(7);
	std::cout << std::endl << "--------- Material ---------" << std::endl;
	std::cout << "Newmtl = '" << print._name << "'" << std::endl;
	std::cout << "Ka = '" << print._ka.x << "' '" << print._ka.y << "' '" << print._ka.z << "'" << std::endl;
	std::cout << "Kd = '" << print._kd.x << "' '" << print._kd.y << "' '" << print._kd.z << "'" << std::endl;
	std::cout << "Ks = '" << print._ks.x << "' '" << print._ks.y << "' '" << print._ks.z << "'" << std::endl;
	std::cout << "Ke = '" << print._ke.x << "' '" << print._ke.y << "' '" << print._ke.z << "'" << std::endl;
	std::cout << "illum = '" << print._illum << "'" << std::endl;
	std::cout << "Ns = '" << print._ns << "'" << std::endl;
	std::cout << "Ni = '" << print._ni << "'" << std::endl;
	std::cout << "d = '" << print._d << "'" << std::endl;
	if (print._mapKa.size())
		std::cout << "mapKa = '" << print._mapKa << "'" << std::endl;
}

std::ostream &  operator<<(std::ostream& o, glm::vec3 vec)
{
    o << vec.x << "," << vec.y << "," << vec.z << "'";
    return o;
}

bool	testObjFile(std::string Path)
{
	if (Path.substr(Path.size() - 4, 4) != ".obj")
	{
		std::cerr << "'" << Path << "' is not a .obj file!" << std::endl;
		return (1);
	}
	return (testOpenFile(Path));
}


bool	testOpenFile(std::string Path)
{
    std::ifstream file(Path);

    if (!file)
	{
		std::cerr << "Failed to open: '" << Path << "'!" << std::endl;
		return (1);
	}
	return (0);
}

bool	isTexFile(std::string file)
{
	std::vector<std::string>			ext{"png", "jpg", "jpeg"};
	bool	add = 0;

	std::string	extFile(file.c_str(), file.find_last_of('.') + 1, file.size() - (file.find_last_of('.') + 1));
	for (std::string authorizedExt : ext) {
		if (authorizedExt == extFile) {
			add = !add;
		}
	}
	if (!add) {
		std::cerr << "This extension is not usable: " << extFile << ". Try with those one ";
		for (std::string authorizedExt : ext)
		{
			std::cerr << authorizedExt;
			if (authorizedExt != ext[ext.size() - 1])
				std::cerr << ", ";
			else
				std::cerr << ".";
		}
		std::cerr << std::endl; 
		return (1);
	}
	return (0);
}

bool	error(std::string msg)
{
	std::cerr << msg << std::endl;
	return (1);
}