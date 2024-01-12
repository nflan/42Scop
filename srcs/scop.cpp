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

#include "../incs/Mesh.hpp"
#include "../incs/Display.hpp"

// #include <string>//
// #include <iostream>//
// #include <fstream>//
// #include <sstream>//
// #include <vector>//
// #include <cstdlib>//
// #include <iomanip>//

// struct  Material {
//     std::string         _newmtl;//name
//     std::vector<double>  _ka;//ambient color
//     std::vector<double>  _kd;//diffuse color
//     std::vector<double>  _ks;//spectacular color
//     std::vector<double>  _ke;//couleur émissive (emissive)
//     bool                _illum;//illum = 1(0) a flat material with no specular highlights, illum = 2(1) denotes the presence of specular highlights
//     double               _ns;//shininess of the material
//     double               _ni;//densite optique
//     double               _d;//transparency d or Tr
//     std::string         _mapKa;//names a file containing a texture map, which should just be an ASCII dump of RGB values
// };

// void	parseKaKdKsKe(std::istringstream rgb, std::vector<double>& toFill)
// {
// 	float	R, G , B;
// 	rgb >> R; rgb >> G; rgb >> B;
// 	toFill.push_back(R);
// 	toFill.push_back(G);
// 	toFill.push_back(B);
// }

// void	printMaterial(Material print)
// {
// 	std::cout << std::endl << "--------- Material ---------" << std::endl;
// 	std::cout << "Newmtl = '" << print._newmtl << "'" << std::endl;
// 	if (print._ka.size())
// 	std::cout << "Ka = '" << print._ka[0] << "' '" << print._ka[1] << "' '" << print._ka[2] << "'" << std::endl;
// 	if (print._kd.size())
// 	std::cout << "Kd = '" << print._kd[0] << "' '" << print._kd[1] << "' '" << print._kd[2] << "'" << std::endl;
// 	if (print._ks.size())
// 	std::cout << "Ks = '" << print._ks[0] << "' '" << print._ks[1] << "' '" << print._ks[2] << "'" << std::endl;
// 	if (print._ke.size())
// 		std::cout << "Ke = '" << print._ke[0] << "' '" << print._ke[1] << "' '" << print._ke[2] << "'" << std::endl;
// 	std::cout << "illum = '" << print._illum << "'" << std::endl;
// 	std::cout << "Ns = '" << print._ns << "'" << std::endl;
// 	std::cout << "Ni = '" << print._ni << "'" << std::endl;
// 	std::cout << "d = '" << print._d << "'" << std::endl;
// 	if (print._mapKa.size())
// 	std::cout << "mapKa = '" << print._mapKa << "'" << std::endl;
// }

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

int main(int ac, char **av)
{
	static_cast<void>(ac);
	parseMtlFile(av[1]);
	Display	app;
	// Mesh	mesh;

	try
	{
		if (ac < 2 || ac > 3)
			throw std::invalid_argument("Invalid number of args!");
		// mesh.LoadObjModel(av[1]);
		app.setFile(av[1]);
		if (ac == 3)
			app.setText(av[2]);
		app.run();
	} 
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}



/*
SWAP CHAIN: Vulkan ne possède pas de concept comme le framebuffer par défaut, et nous devons donc créer une infrastructure qui contiendra les buffers sur lesquels nous effectuerons les rendus avant de les présenter à l'écran. Cette infrastructure s'appelle swap chain sur Vulkan et doit être créée explicitement. La swap chain est essentiellement une file d'attente d'images attendant d'être affichées. Notre application devra récupérer une des images de la file, dessiner dessus puis la retourner à la file d'attente. Le fonctionnement de la file d'attente et les conditions de la présentation dépendent du paramétrage de la swap chain. Cependant, l'intérêt principal de la swap chain est de synchroniser la présentation avec le rafraîchissement de l'écran.
Il y a trois types de propriétés que nous devrons vérifier : voir incs/SwapChain...\.hpp
quelques fonctions qui détermineront les bons paramètres pour obtenir la swap chain la plus efficace possible. Il y a trois types de paramètres à déterminer :

Format de la surface (profondeur de la couleur)
Modes de présentation (conditions de "l'échange" des images avec l'écran)
Swap extent (résolution des images dans la swap chain)
Pour chacun de ces paramètres nous aurons une valeur idéale que nous choisirons si elle est disponible, sinon nous nous rabattrons sur ce qui nous restera de mieux.

Le mode de présentation est clairement le paramètre le plus important pour la swap chain, car il touche aux conditions d'affichage des images à l'écran. Il existe quatre modes avec Vulkan :

VK_PRESENT_MODE_IMMEDIATE_KHR : les images émises par votre application sont directement envoyées à l'écran, ce qui peut produire des déchirures (tearing).
VK_PRESENT_MODE_FIFO_KHR : la swap chain est une file d'attente, et l'écran récupère l'image en haut de la pile quand il est rafraîchi, alors que le programme insère ses nouvelles images à l'arrière. Si la queue est pleine le programme doit attendre. Ce mode est très similaire à la synchronisation verticale utilisée par la plupart des jeux vidéo modernes. L'instant durant lequel l'écran est rafraichi s'appelle l'intervalle de rafraîchissement vertical (vertical blank).
VK_PRESENT_MODE_FIFO_RELAXED_KHR : ce mode ne diffère du précédent que si l'application est en retard et que la queue est vide pendant le vertical blank. Au lieu d'attendre le prochain vertical blank, une image arrivant dans la file d'attente sera immédiatement transmise à l'écran.
VK_PRESENT_MODE_MAILBOX_KHR : ce mode est une autre variation du second mode. Au lieu de bloquer l'application quand le file d'attente est pleine, les images présentes dans la queue sont simplement remplacées par de nouvelles. Ce mode peut être utilisé pour implémenter le triple buffering, qui vous permet d'éliminer le tearing tout en réduisant le temps de latence entre le rendu et l'affichage qu'une file d'attente implique.
Seul VK_PRESENT_MODE_FIFO_KHR est toujours disponible. Nous aurons donc encore à écrire une fonction pour réaliser un choix, car le mode que nous choisirons préférentiellement est VK_PRESENT_MODE_MAILBOX_KHR :
*/
