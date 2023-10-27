#ifndef UNIFORMBUFFEROBJECT_HPP
#define UNIFORMBUFFEROBJECT_HPP

#include </mnt/nfs/homes/nflan/sgoinfre/bin/glm/glm/glm.hpp>


struct UniformBufferObject {
    glm::mat4   model;
    glm::mat4   view;
    glm::mat4   proj;
};

#endif