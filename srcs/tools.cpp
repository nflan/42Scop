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