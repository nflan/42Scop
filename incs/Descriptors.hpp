/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Descriptors.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/08 19:24:19 by nflan             #+#    #+#             */
/*   Updated: 2024/02/08 19:25:11 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DESCRIPTORS_HPP
#define DESCRIPTORS_HPP

#include "Device.hpp"
#include "SwapChain.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>
#include <array>

class ft_DescriptorSetLayout {
	public:
		class Builder {
			public:
				Builder(ft_Device &Device) : _device{Device} {}

				Builder&    addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
				std::unique_ptr<ft_DescriptorSetLayout> build() const;

			private:
				ft_Device&                                                  _device;
				std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>  _bindings{};
		};

		ft_DescriptorSetLayout(ft_Device &Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~ft_DescriptorSetLayout();
		ft_DescriptorSetLayout(const ft_DescriptorSetLayout &) = delete;
		ft_DescriptorSetLayout &operator=(const ft_DescriptorSetLayout &) = delete;

		VkDescriptorSetLayout   getDescriptorSetLayout() const { return _descriptorSetLayout; }

	private:
		ft_Device&                                                  _device;
		VkDescriptorSetLayout                                       _descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>  _bindings;

		friend class ft_DescriptorWriter;
};

class ft_DescriptorPool {
	public:
		class Builder {
			public:
				Builder(ft_Device &Device) : _device{Device} {};

				Builder &addPool(std::array<VkDescriptorPoolSize, 2>);
				Builder &addPoolSize(VkDescriptorType, uint32_t);
				Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
				Builder &setMaxSets(uint32_t count);
				std::unique_ptr<ft_DescriptorPool>  build();

			private:
				ft_Device&                          _device;
				std::vector<VkDescriptorPoolSize>   _poolSizes{};
				uint32_t                            _maxSets = 1000;
				VkDescriptorPoolCreateFlags         _poolFlags = 0;
		};

		ft_DescriptorPool(ft_Device &Device,uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes);
		~ft_DescriptorPool();
		ft_DescriptorPool(const ft_DescriptorPool &) = delete;
		ft_DescriptorPool &operator=(const ft_DescriptorPool &) = delete;

		bool    allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& set);
		VkDescriptorPool    getDescriptorPool() { return _descriptorPool; }
		void    freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

		void    resetPool();

		ft_Device&  getDevice() { return _device; }

	private:
		ft_Device&          _device;
		VkDescriptorPool    _descriptorPool;

		friend class ft_DescriptorWriter;
};

class ft_DescriptorWriter {
	public:
		ft_DescriptorWriter(ft_DescriptorSetLayout &setLayout, ft_DescriptorPool &pool);

		ft_DescriptorWriter    &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
		ft_DescriptorWriter    &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
		ft_DescriptorWriter    &writeImageBuffer(VkDescriptorBufferInfo *bufferInfo, VkDescriptorImageInfo *imageInfo, std::vector<VkDescriptorSet> descriptorSet);

		bool build(VkDescriptorSet &descriptorSet);
		void overwrite(VkDescriptorSet &set);

	private:
		ft_DescriptorSetLayout&             _setLayout;
		ft_DescriptorPool&                  _pool;
		std::vector<VkDescriptorSet>        _descriptorSets;
		std::vector<VkWriteDescriptorSet>   _writes;
};

#endif
