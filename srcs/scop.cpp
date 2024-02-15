/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scop.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/24 15:51:36 by nflan             #+#    #+#             */
/*   Updated: 2023/10/27 12:18:21 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>

#include "../incs/Display.hpp"

int	main(int ac, char **av)
{
	try
	{
		if (ac < 2 || ac > 3)
		{
			std::cerr << "Invalid number of args!";
			return (1);
		}

		if (testObjFile(av[1]))
			return (1);

		Display	app; 
		app.setFile(av[1]);
		if (ac == 3)
			app.setText(av[2]);
		return (app.run());
	} 
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}