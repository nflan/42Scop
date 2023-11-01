#ifndef UNIFORMBUFFEROBJECT_HPP
#define UNIFORMBUFFEROBJECT_HPP

#include </home/nflan/bin/glm/glm/glm.hpp>


struct UniformBufferObject {
    alignas(16) glm::mat4   model;
    alignas(16) glm::mat4   view;
    alignas(16) glm::mat4   proj;
};

#endif