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