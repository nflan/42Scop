/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QueueFamilyIndices.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/25 13:23:00 by nflan             #+#    #+#             */
/*   Updated: 2023/10/25 14:52:58 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUEUEFAMILYINDICES_HPP
#define QUEUEFAMILYINDICES_HPP

#include <stdint.h>
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t>	graphicsFamily;
	std::optional<uint32_t>	presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

#endif
