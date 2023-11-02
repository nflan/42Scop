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

int main(int ac, char **av)
{
	Display	app;
	// Mesh	mesh;

	try
	{
		if (ac != 2)
			throw std::invalid_argument("Invalid number of args!");
		// mesh.LoadObjModel(av[1]);
		app.setFile(av[1]);
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
