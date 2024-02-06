#include "../incs/Descriptors.hpp"
#include <iostream>

// std
#include <cassert>
#include <stdexcept>

extern int RENDER;

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
    
    for (std::pair<uint32_t, VkDescriptorSetLayoutBinding> kv : bindings)
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
    return std::make_unique<ft_DescriptorPool>(this->_device, this->_maxSets, this->_poolFlags, this->_poolSizes);
}

// *************** Descriptor Pool *********************

ft_DescriptorPool::ft_DescriptorPool(ft_Device &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes): _device(device)
{	
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());;
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(this->_device.device(), &descriptorPoolInfo, nullptr, &this->_descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool!");
}

ft_DescriptorPool::~ft_DescriptorPool()
{
    vkDestroyDescriptorPool(this->_device.device(), this->_descriptorPool, nullptr);
}

bool ft_DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& set)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool w henever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(this->_device.device(), &allocInfo, &set) != VK_SUCCESS)
        return false;
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

ft_DescriptorWriter::ft_DescriptorWriter(ft_DescriptorSetLayout &setLayout, ft_DescriptorPool &pool): _setLayout(setLayout), _pool(pool) {}

ft_DescriptorWriter    &ft_DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
{
    assert(this->_setLayout._bindings.count(binding) == 1 && "Layout does not contain specified binding");

    VkDescriptorSetLayoutBinding    &bindingDescription = this->_setLayout._bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet    write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    this->_writes.push_back(write);
    return *this;
}

ft_DescriptorWriter    &ft_DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo)
{
    assert(this->_setLayout._bindings.count(binding) == 1 && "Layout does not contain specified binding");

    VkDescriptorSetLayoutBinding    &bindingDescription = this->_setLayout._bindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet    write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    this->_writes.push_back(write);
    return *this;
}

bool    ft_DescriptorWriter::build(VkDescriptorSet &set)
{
    bool success = this->_pool.allocateDescriptor(this->_setLayout.getDescriptorSetLayout(), set);
    if (!success)
        return false;

    overwrite(set);
    return true;
}

void    ft_DescriptorWriter::overwrite(VkDescriptorSet &set)
{
    for (auto &write : _writes)
        write.dstSet = set;
    vkUpdateDescriptorSets(this->_pool.getDevice().device(), this->_writes.size(), this->_writes.data(), 0, nullptr);
}