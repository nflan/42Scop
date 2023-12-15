#include "../incs/Descriptors.hpp"
#include <iostream>

// std
#include <cassert>
#include <stdexcept>

// *************** Descriptor Set Layout Builder *********************

ft_DescriptorSetLayout::Builder &ft_DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count)
{
    assert(this->_bindings.count(binding) == 0 && "Binding already in use");

    VkDescriptorSetLayoutBinding  layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.pImmutableSamplers = nullptr; // Optionnel

    this->_bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<ft_DescriptorSetLayout> ft_DescriptorSetLayout::Builder::build() const
{
    return std::make_unique<ft_DescriptorSetLayout>(this->_device, this->_bindings);
}

// *************** Descriptor Set Layout *********************

ft_DescriptorSetLayout::ft_DescriptorSetLayout(ft_Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings): _device{device}, _bindings{bindings}
{
    std::vector<VkDescriptorSetLayoutBinding>   setLayoutBindings{};
    
    for (auto kv : bindings)
        setLayoutBindings.push_back(kv.second);

    std::cout << "setlayoutbindings size = " << setLayoutBindings.size() << std::endl;
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    layoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(this->_device.device(), &layoutInfo, nullptr, &this->_descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
}

ft_DescriptorSetLayout::~ft_DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(this->_device.device(), this->_descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

ft_DescriptorPool::Builder &ft_DescriptorPool::Builder::addPool(std::array<VkDescriptorPoolSize, 2> descriptorType)
{
    for (VkDescriptorPoolSize pool : descriptorType)
        this->_poolSizes.push_back(pool);
    // this->_poolSizes.push_back(descriptorType);
    return *this;
}

ft_DescriptorPool::Builder &ft_DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
    this->_poolSizes.push_back({descriptorType, count});
    return *this;
}

ft_DescriptorPool::Builder &ft_DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    this->_poolFlags = flags;
    return *this;
}

ft_DescriptorPool::Builder &ft_DescriptorPool::Builder::setMaxSets(uint32_t count)
{
    this->_maxSets = count;
    return *this;
}

std::unique_ptr<ft_DescriptorPool> ft_DescriptorPool::Builder::build()
{
    return std::make_unique<ft_DescriptorPool>(this->_device, this->_maxSets, this->_poolFlags, this->_poolSizes, this->_swapChain);
}

// *************** Descriptor Pool *********************

ft_DescriptorPool::ft_DescriptorPool(ft_Device &Device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, std::vector<VkDescriptorPoolSize> &poolSizess, ft_SwapChain &swapChain): _device(Device), _swapChain(swapChain)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChain.imageCount());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChain.imageCount());
	
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());;
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = static_cast<uint32_t>(swapChain.imageCount());
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(this->_device.device(), &descriptorPoolInfo, nullptr, &this->_descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool!");
}

ft_DescriptorPool::~ft_DescriptorPool()
{
    vkDestroyDescriptorPool(this->_device.device(), this->_descriptorPool, nullptr);
}

bool ft_DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, std::vector<VkDescriptorSet> &set)
{
    SwapChainSupportDetails	swapChainSupport = this->_device.querySwapChainSupport(this->_device.getPhysicalDevice());
	std::vector<VkDescriptorSetLayout> layouts(this->_swapChain.imageCount(), descriptorSetLayout);

    // std::cout << swapChainSupport.capabilities.minImageCount + 1 << std::endl;
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->_descriptorPool;
    allocInfo.descriptorSetCount = this->_swapChain.imageCount();//static_cast<uint32_t>(ft_SwapChain::MAX_FRAMES_IN_FLIGHT);//static_cast<uint32_t>(this->_swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();
    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool w henever an old pool fills up. But this is beyond our current scope
    std::cerr << set.size() << std::endl;
    set.resize(this->_swapChain.imageCount());
    std::cerr << set.size() << std::endl;
    if (vkAllocateDescriptorSets(this->_device.device(), &allocInfo, set.data()) != VK_SUCCESS)
        return false;
    std::cerr << set.size() << std::endl;

    return true;
}

void ft_DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
{
    vkFreeDescriptorSets(this->_device.device(), this->_descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void ft_DescriptorPool::resetPool()
{
    vkResetDescriptorPool(this->_device.device(), this->_descriptorPool, 0);
}

// *************** Descriptor Writer *********************

ft_DescriptorWriter::ft_DescriptorWriter(ft_DescriptorSetLayout &setLayout, ft_DescriptorPool &pool, ft_Device &device): _setLayout(setLayout), _pool(pool), _device(device) {}

ft_DescriptorWriter    &ft_DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
{
    assert(this->_setLayout._bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = this->_setLayout._bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet    write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    this->_writes.push_back(write);
    return *this;
}

ft_DescriptorWriter    &ft_DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo)
{
    assert(this->_setLayout._bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = this->_setLayout._bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet    write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    this->_writes.push_back(write);
    return *this;
}

ft_DescriptorWriter    &ft_DescriptorWriter::writeImageBuffer(VkDescriptorBufferInfo *bufferInfo, VkDescriptorImageInfo *imageInfo, std::vector<VkDescriptorSet> descriptorSet)
{
    _descriptorSets = descriptorSet;
    this->build();
    descriptorSet = _descriptorSets;
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet[0];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet[1];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = imageInfo;

    vkUpdateDescriptorSets(this->_device.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    this->_writes.push_back(descriptorWrites[0]);
    this->_writes.push_back(descriptorWrites[1]);
    return *this;
}

bool    ft_DescriptorWriter::build()
{
    bool success = this->_pool.allocateDescriptor(this->_setLayout.getDescriptorSetLayout(), this->_descriptorSets);
    if (!success)
        return false;

    // overwrite(this->_descriptorSets);
    return true;
}

void    ft_DescriptorWriter::overwrite(std::vector<VkDescriptorSet> set)
{
    for (uint32_t i = 0; i < _writes.size(); i++)
        _writes[i].dstSet = set[i];
    vkUpdateDescriptorSets(this->_pool.getDevice().device(), this->_writes.size(), this->_writes.data(), 0, nullptr);
}