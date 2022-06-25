#include "graphmodule.hpp"


void GraphModule::prepareGraphics()
{
    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &graphics.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &graphics.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));
}


void GraphModule::buildCommandBuffers(){}


void GraphModule::loadAssets()
{
    //const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    //nGraphics.models.rock.loadFromFile("../data/models/rock01.gltf", vulkanDevice, queue, glTFLoadingFlags); //getAssetPath() +
    //nGraphics.textures.rocks.loadFromFile("../data/textures/texturearray_rocks_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
    //nGraphics.textures.rocks.loadFromFile(getAssetPath() + "textures/vulkan_11_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
    //graphics.textures.textureColorMap.loadFromFile(getAssetPath() + "textures/vulkan_11_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

   // ndearImgui.imageTemp=ndearImgui.registerTexture(graphics.textures.textureColorMap);

    generateCircle();
    generateQuad();
    generateQuadSimple();
}

void GraphModule::setupDescriptorPool()
{
    // Example uses one ubo
    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 15),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5),
        // Compute pipelines uses a storage image for image reads and writes
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5),
    };

   VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 33);
   //VkDescriptorPoolCreateInfo descriptorPoolInfo =
   //    vks::initializers::descriptorPoolCreateInfo(poolSizes.size(),poolSizes.data(),3);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void GraphModule::setupDescriptorSetLayout()
{
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            // Binding 0 : Vertex shader uniform buffer
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
            // Binding 1 : Fragment shader combined sampler
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
            // Binding 0 : Vertex shader uniform buffer
            //vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 2),
            // Binding 1 : Fragment shader combined sampler
            //vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &nGraphics.descriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&nGraphics.descriptorSetLayout, 1);
        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &nGraphics.pipelineLayout));
    }
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            // Binding 0 : Vertex shader uniform buffer
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
            // Binding 1 : Fragment shader combined sampler
            vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
            // Binding 0 : Vertex shader uniform buffer
            //vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 2),
            // Binding 1 : Fragment shader combined sampler
            //vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &mapGraphics.descriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&mapGraphics.descriptorSetLayout, 1);
        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &mapGraphics.pipelineLayout));

        //VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &mapGraphics.pipelineLayout));
    }


}

void GraphModule::setupDescriptorSet()
{
    {
        VkDescriptorSetAllocateInfo descripotrSetAllocInfo;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

        descripotrSetAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &nGraphics.descriptorSetLayout, 1);;

        // Instanced rocks
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &nGraphics.descriptorSets.instancedParticles));
        writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(nGraphics.descriptorSets.instancedParticles, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &nGraphics.uniformBuffers2.scene.descriptor),	// Binding 0 : Vertex shader uniform buffer
            vks::initializers::writeDescriptorSet(nGraphics.descriptorSets.instancedParticles, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,&graphics.textures.textureComputeTarget.descriptor)	// Binding 1 : Color map
        };
        vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
    }

    {
        VkDescriptorSetAllocateInfo descripotrSetAllocInfo;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

        descripotrSetAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &nGraphics.descriptorSetLayout, 1);;

        // Instanced rocks
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &nGraphics.descriptorSets.instancedConstraints));
        writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(nGraphics.descriptorSets.instancedConstraints, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &nGraphics.uniformBuffers2.scene.descriptor),	// Binding 0 : Vertex shader uniform buffer
            vks::initializers::writeDescriptorSet(nGraphics.descriptorSets.instancedConstraints, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,&graphics.textures.textureComputeTarget.descriptor)	// Binding 1 : Color map
        };
        vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
    }

    {
        VkDescriptorSetAllocateInfo descripotrSetAllocInfo;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

        descripotrSetAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &nGraphics.descriptorSetLayout, 1);;

        // Instanced rocks
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &nGraphics.descriptorSets.drawConstraints));
        writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(nGraphics.descriptorSets.drawConstraints, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &nGraphics.uniformBuffers2.constraint_scene.descriptor)	// Binding 0 : Vertex shader uniform buffer
           // vks::initializers::writeDescriptorSet(nGraphics.descriptorSets.drawConstraints, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &graphics.textures.textureComputeTarget.descriptor)	// Binding 1 : Color map
        };
        vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
    }

    {
        VkDescriptorSetAllocateInfo descripotrSetAllocInfo;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

        descripotrSetAllocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &mapGraphics.descriptorSetLayout, 1);;

        // Instanced rocks
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descripotrSetAllocInfo, &mapGraphics.descriptorSet));
        writeDescriptorSets = {
            vks::initializers::writeDescriptorSet(mapGraphics.descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &nGraphics.uniformBuffers2.scene.descriptor),	// Binding 0 : Vertex shader uniform buffer
        };
        vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
    }

}

void GraphModule::prepareParticlePipeline()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState =vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(nGraphics.pipelineLayout, renderPass);
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();

    // This example uses two different input states, one for the instanced part and one for non-instanced rendering
    VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    // Vertex input bindings
    // The instancing pipeline uses a vertex input state with two bindings
    bindingDescriptions = {
        // Binding point 0: Mesh vertex layout description at per-vertex rate
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(VertexPosNormUv), VK_VERTEX_INPUT_RATE_VERTEX),
        // Binding point 1: Instanced data at per-instance rate
        vks::initializers::vertexInputBindingDescription(INSTANCE_BUFFER_BIND_ID, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
    };

    // Vertex attribute bindings
    // Note that the shader declaration for per-vertex and per-instance attributes is the same, the different input rates are only stored in the bindings:
    // instanced.vert:
    //	layout (location = 0) in vec3 inPos;		Per-Vertex
    //	...
    //	layout (location = 4) in vec3 instancePos;	Per-Instance
    attributeDescriptions = {
        // Per-vertex attributes
        // These are advanced for each vertex fetched by the vertex shader
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Location 0: Position
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT,    sizeof(glm::vec3) ),	// Location 1: Normal
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32_SFLOAT,       sizeof(glm::vec3) * 2),		// Location 2: Texture coordinates
       // vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),	// Location 3: Color
        // Per-Instance attributes
        // These are fetched for each instance rendered
        vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32_SFLOAT, 0),					// Location 4: Position
        vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 4, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2)),	// Location 5: Rotation
        vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 5, VK_FORMAT_R32_SFLOAT,    sizeof(glm::vec2) * 2),			// Location 6: Scale
        vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 6, VK_FORMAT_R32_SINT,      sizeof(float) * 5),			// Location 7: Texture array layer index
    };
    inputState.pVertexBindingDescriptions = bindingDescriptions.data();
    inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineCI.pVertexInputState = &inputState;

    // Instancing pipeline
    shaderStages[0] = loadShader("../data/shaders/render/instancing_s.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("../data/shaders/render/instancing_s.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    // Use all input bindings and attribute descriptions
    inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &nGraphics.pipelines.instancedParticles));

}


void GraphModule::prepareConstraintsPipeline()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState =vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(nGraphics.pipelineLayout, renderPass);
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();

    // This example uses two different input states, one for the instanced part and one for non-instanced rendering
    VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    // Vertex input bindings
    // The instancing pipeline uses a vertex input state with two bindings
    bindingDescriptions = {
        // Binding point 0: Mesh vertex layout description at per-vertex rate
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX),

    };

    // Vertex attribute bindings
    // Note that the shader declaration for per-vertex and per-instance attributes is the same, the different input rates are only stored in the bindings:
    // instanced.vert:
    //	layout (location = 0) in vec3 inPos;		Per-Vertex
    //	...
    //	layout (location = 4) in vec3 instancePos;	Per-Instance
    attributeDescriptions = {
        // Per-vertex attributes
        // These are advanced for each vertex fetched by the vertex shader
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0,  VK_FORMAT_R32G32_SFLOAT, 0)
    };
    inputState.pVertexBindingDescriptions = bindingDescriptions.data();
    inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineCI.pVertexInputState = &inputState;

    // Instancing pipeline
    shaderStages[0] = loadShader("../data/shaders/render/instancing_c.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("../data/shaders/render/instancing_c.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    // Use all input bindings and attribute descriptions
    inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &nGraphics.pipelines.instancedConstraints));

}

void GraphModule::prepareDrawConstraintsPipeline()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState =vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(mapGraphics.pipelineLayout, renderPass);
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();

    // This example uses two different input states, one for the instanced part and one for non-instanced rendering
    VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    // Vertex input bindings
    // The instancing pipeline uses a vertex input state with two bindings
    bindingDescriptions = {
        // Binding point 0: Mesh vertex layout description at per-vertex rate
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX),

    };

    // Vertex attribute bindings
    attributeDescriptions = {
        // Per-vertex attributes
        // These are advanced for each vertex fetched by the vertex shader
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0,  VK_FORMAT_R32G32_SFLOAT, 0)
    };
    inputState.pVertexBindingDescriptions = bindingDescriptions.data();
    inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineCI.pVertexInputState = &inputState;

    // Instancing pipeline
    shaderStages[0] = loadShader("../data/shaders/render/render_constraints.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("../data/shaders/render/render_constraints.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    // Use all input bindings and attribute descriptions
    inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &nGraphics.pipelines.drawConstraints));

}


void GraphModule::prepareDrawPolylinePipeline()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState =vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(mapGraphics.pipelineLayout, renderPass);
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();

    // This example uses two different input states, one for the instanced part and one for non-instanced rendering
    VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    // Vertex input bindings
    // The instancing pipeline uses a vertex input state with two bindings
    bindingDescriptions = {
        // Binding point 0: Mesh vertex layout description at per-vertex rate
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX),
    };

    // Vertex attribute bindings
    attributeDescriptions = {
        // Per-vertex attributes
        // These are advanced for each vertex fetched by the vertex shader
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0,  VK_FORMAT_R32G32_SFLOAT, 0)
    };
    inputState.pVertexBindingDescriptions = bindingDescriptions.data();
    inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineCI.pVertexInputState = &inputState;

    // Instancing pipeline
    shaderStages[0] = loadShader("../data/shaders/render/render_constraints.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("../data/shaders/render/render_constraints.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    // Use all input bindings and attribute descriptions
    inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &nGraphics.pipelines.drawPolyline));

}


void GraphModule::prepareDrawTrianglePipeline()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState =vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(mapGraphics.pipelineLayout, renderPass);
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();

    // This example uses two different input states, one for the instanced part and one for non-instanced rendering
    VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    // Vertex input bindings
    // The instancing pipeline uses a vertex input state with two bindings
    bindingDescriptions = {
        // Binding point 0: Mesh vertex layout description at per-vertex rate
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(TrianglesDrawStruct), VK_VERTEX_INPUT_RATE_VERTEX),
    };

    // Vertex attribute bindings
    attributeDescriptions = {
        // Per-vertex attributes
        // These are advanced for each vertex fetched by the vertex shader
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0,  VK_FORMAT_R32G32_SFLOAT, 0),
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1,  VK_FORMAT_R32G32B32A32_SFLOAT,sizeof(glm::vec2) )
    };
    inputState.pVertexBindingDescriptions = bindingDescriptions.data();
    inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineCI.pVertexInputState = &inputState;

    // Instancing pipeline
    shaderStages[0] = loadShader("../data/shaders/render/render_triangles.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("../data/shaders/render/render_triangles.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    // Use all input bindings and attribute descriptions
    inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &nGraphics.pipelines.drawTriangles));

}



void GraphModule::prepareMapPipeline()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState =vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(mapGraphics.pipelineLayout, renderPass);
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();

    // This example uses two different input states, one for the instanced part and one for non-instanced rendering
    VkPipelineVertexInputStateCreateInfo inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    // Vertex input bindings
    // The instancing pipeline uses a vertex input state with two bindings
    bindingDescriptions = {
        // Binding point 0: Mesh vertex layout description at per-vertex rate
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX),
        // Binding point 1: Instanced data at per-instance rate
        vks::initializers::vertexInputBindingDescription(INSTANCE_BUFFER_BIND_ID, sizeof(int), VK_VERTEX_INPUT_RATE_INSTANCE)
    };

    // Vertex attribute bindings
    // Note that the shader declaration for per-vertex and per-instance attributes is the same, the different input rates are only stored in the bindings:
    // instanced.vert:
    //	layout (location = 0) in vec3 inPos;		Per-Vertex
    //	...
    //	layout (location = 4) in vec3 instancePos;	Per-Instance
    attributeDescriptions = {
        // Per-vertex attributes
        // These are advanced for each vertex fetched by the vertex shader
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Location 0: Position
        // Per-Instance attributes
        // These are fetched for each instance rendered
        vks::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 1, VK_FORMAT_R32_SINT, 0),			// Location 7: Texture array layer index
    };
    inputState.pVertexBindingDescriptions = bindingDescriptions.data();
    inputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    pipelineCI.pVertexInputState = &inputState;

    // Instancing pipeline
    shaderStages[0] = loadShader("../data/shaders/render/vs_boxes.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("../data/shaders/render/fs_boxes.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    // Use all input bindings and attribute descriptions
    inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &mapGraphics.pipeline));

}



void GraphModule::buildCommandBuffer(int i)
{
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    ndearImgui.preRender(MainGUI, simCommandParams, simDynamicParametrs, getPlatform());

    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.125f, 0.125f, 0.125f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

    // Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // We won't be changing the layout of the image
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageMemoryBarrier.image = graphics.textures.textureComputeTarget.image;
    imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkCmdPipelineBarrier(
        drawCmdBuffers[i],
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);


    //offSreen Render
    {
            /*
            VkClearValue clearValues[2];
            clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f, 0 };
            */

            VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
            renderPassBeginInfo.renderPass = offscreenPass.renderPass;
            renderPassBeginInfo.framebuffer = offscreenPass.frameBuffer;
            renderPassBeginInfo.renderArea.extent.width = offscreenPass.width;
            renderPassBeginInfo.renderArea.extent.height = offscreenPass.height;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;

            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = vks::initializers::viewport((float)offscreenPass.width, (float)offscreenPass.height, 0.0f, 1.0f);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
            VkRect2D scissor = vks::initializers::rect2D(offscreenPass.width, offscreenPass.height, 0, 0);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
            VkDeviceSize offsets[1] = { 0 };

            // Instanced rocks
            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mapGraphics.pipelineLayout, 0, 1, &mapGraphics.descriptorSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mapGraphics.pipeline);
            // Binding point 0 : Mesh vertex buffer
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &mapGraphics.quard.vertexBuffer.buffer, offsets);
            // Binding point 1 : Instance data buffer
            vkCmdBindVertexBuffers(drawCmdBuffers[i], INSTANCE_BUFFER_BIND_ID, 1, &computeBlock.buffers.mapSize.buffer, offsets);
            // Bind index buffer
            vkCmdBindIndexBuffer(drawCmdBuffers[i], mapGraphics.quard.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            // Render instances
            vkCmdDrawIndexed(drawCmdBuffers[i], mapGraphics.quard.indexCount, solver.mapElementsAmount, 0, 0, 0);
            vkCmdEndRenderPass(drawCmdBuffers[i]);
    }
/*
    //offSreen Render Constraints
    {

            VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
            renderPassBeginInfo.renderPass =                offscreenPassConstraints.renderPass;
            renderPassBeginInfo.framebuffer =               offscreenPassConstraints.frameBuffer;
            renderPassBeginInfo.renderArea.extent.width =   offscreenPassConstraints.width;
            renderPassBeginInfo.renderArea.extent.height =  offscreenPassConstraints.height;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;

            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = vks::initializers::viewport((float)offscreenPassConstraints.width, (float)offscreenPassConstraints.height, 0.0f, 1.0f);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
            VkRect2D scissor = vks::initializers::rect2D(offscreenPassConstraints.width, offscreenPassConstraints.height, 0, 0);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
            VkDeviceSize offsets[1] = { 0 };

            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mapGraphics.pipelineLayout, 0, 1, &nGraphics.descriptorSets.drawConstraints, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelines.drawConstraints);
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.instanceConstraints.buffer, offsets);
            vkCmdSetLineWidth(drawCmdBuffers[i],simCommandParams.lineWidth);
            vkCmdDraw(drawCmdBuffers[i], 2*simDynamicParametrs.constraintsNum ,1, 0, 0); //need number constraints

            vkCmdEndRenderPass(drawCmdBuffers[i]);
    }
*/

    //offSreen Render Constraints
    //if(simCommandParams.drawPolygons)
    {
            VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
            renderPassBeginInfo.renderPass =                offscreenPassConstraints.renderPass;
            renderPassBeginInfo.framebuffer =               offscreenPassConstraints.frameBuffer;
            renderPassBeginInfo.renderArea.extent.width =   offscreenPassConstraints.width;
            renderPassBeginInfo.renderArea.extent.height =  offscreenPassConstraints.height;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;

            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = vks::initializers::viewport((float)offscreenPassConstraints.width, (float)offscreenPassConstraints.height, 0.0f, 1.0f);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
            VkRect2D scissor = vks::initializers::rect2D(offscreenPassConstraints.width, offscreenPassConstraints.height, 0, 0);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
            VkDeviceSize offsets[1] = { 0 };

/*

            if(simCommandParams.drawTriangles)
            {
                vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.drawTriangle.buffer, offsets);
                for(int ip=0;ip<solver.areaList.size();ip++)
                {
                    //nGraphics.drawPolyline.map(30*sizeof(glm::vec2));
                    auto polygon=solver.triangles_draw_vertex[solver.areaList[ip].first];
                    if(polygon.size()>0)
                    {
                        vector<TrianglesDrawStruct> points;// { glm::vec2(100,100),glm::vec2(150,150),glm::vec2(250,50),glm::vec2(100,100)};
                        for(auto p:polygon) {
                            points.push_back(p);

                        }

                        //points.resize(polygon.p.size());
                        //memcpy(nGraphics.drawPolyline.mapped, points.data(),30*sizeof(glm::vec2));
                        //nGraphics.drawPolyline.unmap();

                        vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mapGraphics.pipelineLayout, 0, 1, &nGraphics.descriptorSets.drawConstraints, 0, NULL);
                        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelines.drawTriangles);

                        vkCmdUpdateBuffer(drawCmdBuffers[i], nGraphics.drawTriangle.buffer,0, points.size()*sizeof(TrianglesDrawStruct), points.data());

                        //vkCmdSetLineWidth(drawCmdBuffers[i],simCommandParams.lineWidth);
                        vkCmdDraw(drawCmdBuffers[i], points.size(),1, 0, 0); //need number constraints

                        vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.drawTriangle.buffer, offsets);

                    }
                }
            }


            if(simCommandParams.drawPolygons)
            {

                vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.drawPolyline.buffer, offsets);
                for(int ip=0;ip<solver.polygons_draw.size();ip++)
                {
                    //nGraphics.drawPolyline.map(30*sizeof(glm::vec2));
                    auto polygon=solver.polygons_draw[ip];
                    if(polygon.size()>0)
                    {
                        vector<glm::vec2> points;// { glm::vec2(100,100),glm::vec2(150,150),glm::vec2(250,50),glm::vec2(100,100)};

                        for(auto p:polygon)
                        {
                            points.push_back(p);

                        }
                        points.push_back(polygon[0]);

                        //points.resize(polygon.p.size());
                        //memcpy(nGraphics.drawPolyline.mapped, points.data(),30*sizeof(glm::vec2));
                        //nGraphics.drawPolyline.unmap();

                        vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mapGraphics.pipelineLayout, 0, 1, &nGraphics.descriptorSets.drawConstraints, 0, NULL);
                        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelines.drawPolyline);

                        vkCmdUpdateBuffer(drawCmdBuffers[i], nGraphics.drawPolyline.buffer,0, points.size()*sizeof(glm::vec2), points.data());

                        vkCmdSetLineWidth(drawCmdBuffers[i],simCommandParams.lineWidth);
                        vkCmdDraw(drawCmdBuffers[i], points.size(),1, 0, 0); //need number constraints

                        vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.drawPolyline.buffer, offsets);

                    }
                }
            }


*/

            vkCmdEndRenderPass(drawCmdBuffers[i]);

           // simCommandParams.runLoop=true;
           // simCommandParams.readPolygons=true;

    }

    //offSreen Render Particles
    {
            /*
            VkClearValue clearValues[2];
            clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f, 0 };
            */

            VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
            renderPassBeginInfo.renderPass =                offscreenPassParticles.renderPass;
            renderPassBeginInfo.framebuffer =               offscreenPassParticles.frameBuffer;
            renderPassBeginInfo.renderArea.extent.width =   offscreenPassParticles.width;
            renderPassBeginInfo.renderArea.extent.height =  offscreenPassParticles.height;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;

            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = vks::initializers::viewport((float)offscreenPassParticles.width, (float)offscreenPassParticles.height, 0.0f, 1.0f);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
            VkRect2D scissor = vks::initializers::rect2D(offscreenPassParticles.width, offscreenPassParticles.height, 0, 0);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
            VkDeviceSize offsets[1] = { 0 };



            if(simCommandParams.drawParticles)
            {
                // Instanced rocks
                vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelineLayout, 0, 1, &nGraphics.descriptorSets.instancedParticles, 0, NULL);
                vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelines.instancedParticles);
                // Binding point 0 : Mesh vertex buffer
                vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.circle.vertexBuffer.buffer, offsets);
                // Binding point 1 : Instance data buffer
                vkCmdBindVertexBuffers(drawCmdBuffers[i], INSTANCE_BUFFER_BIND_ID, 1, &nGraphics.instanceBuffer.buffer, offsets);
                // Bind index buffer
                vkCmdBindIndexBuffer(drawCmdBuffers[i], nGraphics.circle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                // Render instances
                vkCmdDrawIndexed(drawCmdBuffers[i], nGraphics.circle.indexCount, simParams.numParticles, 0, 0, 0);
            }
            if(simCommandParams.drawConstraints)
            {
                // Instanced rocks
                vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelineLayout, 0, 1, &nGraphics.descriptorSets.instancedConstraints, 0, NULL);
                vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelines.instancedConstraints);
                // Binding point 0 : Mesh vertex buffer
                vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.instanceConstraints.buffer, offsets);
                // Binding point 1 : Instance data buffer
                //vkCmdBindVertexBuffers(drawCmdBuffers[i], INSTANCE_BUFFER_BIND_ID, 1, &nGraphics.instanceConstraints.buffer, offsets);
                // Bind index buffer
                //vkCmdBindIndexBuffer(drawCmdBuffers[i], nGraphics.circle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                // Render instances
                vkCmdSetLineWidth(drawCmdBuffers[i],simCommandParams.lineWidth);
                vkCmdDraw(drawCmdBuffers[i], 2*simDynamicParametrs.constraintsNum ,1, 0, 0); //need number constraints
            }


            vkCmdEndRenderPass(drawCmdBuffers[i]);
    }

    {
        VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.extent.width = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;

        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
        VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
        VkDeviceSize offsets[1] = { 0 };
/*
        if(simCommandParams.drawParticles)
        {
            // Instanced rocks
            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelineLayout, 0, 1, &nGraphics.descriptorSets.instancedParticles, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelines.instancedParticles);
            // Binding point 0 : Mesh vertex buffer
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.circle.vertexBuffer.buffer, offsets);
            // Binding point 1 : Instance data buffer
            vkCmdBindVertexBuffers(drawCmdBuffers[i], INSTANCE_BUFFER_BIND_ID, 1, &nGraphics.instanceBuffer.buffer, offsets);
            // Bind index buffer
            vkCmdBindIndexBuffer(drawCmdBuffers[i], nGraphics.circle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            // Render instances
            vkCmdDrawIndexed(drawCmdBuffers[i], nGraphics.circle.indexCount, simParams.numParticles, 0, 0, 0);
        }

        if(simCommandParams.drawConstraints)
        {
            // Instanced rocks
            //vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelineLayout, 0, 1, &nGraphics.descriptorSets.instancedConstraints, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, nGraphics.pipelines.instancedConstraints);
            // Binding point 0 : Mesh vertex buffer
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &nGraphics.instanceConstraints.buffer, offsets);
            // Binding point 1 : Instance data buffer
            //vkCmdBindVertexBuffers(drawCmdBuffers[i], INSTANCE_BUFFER_BIND_ID, 1, &nGraphics.instanceConstraints.buffer, offsets);
            // Bind index buffer
            //vkCmdBindIndexBuffer(drawCmdBuffers[i], nGraphics.circle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            // Render instances
            vkCmdSetLineWidth(drawCmdBuffers[i],simCommandParams.lineWidth);
            vkCmdDraw(drawCmdBuffers[i], 2*simDynamicParametrs.constraintsNum ,1, 0, 0); //need number constraints
        }
*/
        //drawUI(drawCmdBuffers[i]);


        vkCmdEndRenderPass(drawCmdBuffers[i]);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), drawCmdBuffers[i]);// One command buffer

    }

    VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));

}


//------------------------------------------OffScreenRender-----------------------------------

// Setup the offscreen framebuffer for rendering the mirrored scene
    // The color attachment of this framebuffer will then be used to sample from in the fragment shader of the final pass
void GraphModule::prepareOffscreen()
{
    offscreenPass.width =  SIM_WIDTH;
    offscreenPass.height = SIM_HEIGHT;//(float)(FB_DIM_X)*(float)(height/width);

    // Find a suitable depth format
    VkFormat fbDepthFormat;
    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &fbDepthFormat);
    assert(validDepthFormat);

    // Color attachment
    VkImageCreateInfo image = vks::initializers::imageCreateInfo();
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = FB_COLOR_FORMAT;
    image.extent.width = offscreenPass.width;
    image.extent.height = offscreenPass.height;
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    // We will sample directly from the color attachment
    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &offscreenPass.color.image));
    vkGetImageMemoryRequirements(device, offscreenPass.color.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPass.color.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, offscreenPass.color.image, offscreenPass.color.mem, 0));

    VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = FB_COLOR_FORMAT;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = offscreenPass.color.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &colorImageView, nullptr, &offscreenPass.color.view));

    // Create sampler to sample from the attachment in the fragment shader
    VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &offscreenPass.sampler));

    // Depth stencil attachment
    image.format = fbDepthFormat;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &offscreenPass.depth.image));
    vkGetImageMemoryRequirements(device, offscreenPass.depth.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPass.depth.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, offscreenPass.depth.image, offscreenPass.depth.mem, 0));

    VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = fbDepthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = offscreenPass.depth.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &offscreenPass.depth.view));

    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

    std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
    // Color attachment
    attchmentDescriptions[0].format = FB_COLOR_FORMAT;
    attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Depth attachment
    attchmentDescriptions[1].format = fbDepthFormat;
    attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
    renderPassInfo.pAttachments = attchmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &offscreenPass.renderPass));

    VkImageView attachments[2];
    attachments[0] = offscreenPass.color.view;
    attachments[1] = offscreenPass.depth.view;

    VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
    fbufCreateInfo.renderPass = offscreenPass.renderPass;
    fbufCreateInfo.attachmentCount = 2;
    fbufCreateInfo.pAttachments = attachments;
    fbufCreateInfo.width = offscreenPass.width;
    fbufCreateInfo.height = offscreenPass.height;
    fbufCreateInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offscreenPass.frameBuffer));

    // Fill a descriptor for later use in a descriptor set
    offscreenPass.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    offscreenPass.descriptor.imageView = offscreenPass.color.view;
    offscreenPass.descriptor.sampler = offscreenPass.sampler;
}


//------------------------------------------OffScreenRenderConstraints-----------------------------------

// Setup the offscreen framebuffer for rendering the mirrored scene
    // The color attachment of this framebuffer will then be used to sample from in the fragment shader of the final pass
void GraphModule::prepareOffscreenRenderConstraints()
{
    offscreenPassConstraints.width =  SIM_WIDTH;
    offscreenPassConstraints.height = SIM_HEIGHT;//(float)(FB_DIM_X)*(float)(height/width);

    // Find a suitable depth format
    VkFormat fbDepthFormat;
    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &fbDepthFormat);
    assert(validDepthFormat);

    // Color attachment
    VkImageCreateInfo image = vks::initializers::imageCreateInfo();
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = FB_COLOR_FORMAT;
    image.extent.width =  offscreenPassConstraints.width;
    image.extent.height = offscreenPassConstraints.height;
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    // We will sample directly from the color attachment
    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT| VK_IMAGE_USAGE_STORAGE_BIT;

    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &offscreenPassConstraints.color.image));
    vkGetImageMemoryRequirements(device, offscreenPassConstraints.color.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPassConstraints.color.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, offscreenPassConstraints.color.image, offscreenPassConstraints.color.mem, 0));

    VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = FB_COLOR_FORMAT;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = offscreenPassConstraints.color.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &colorImageView, nullptr, &offscreenPassConstraints.color.view));

    // Create sampler to sample from the attachment in the fragment shader
    VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &offscreenPassConstraints.sampler));

    // Depth stencil attachment
    image.format = fbDepthFormat;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &offscreenPassConstraints.depth.image));
    vkGetImageMemoryRequirements(device, offscreenPassConstraints.depth.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPassConstraints.depth.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, offscreenPassConstraints.depth.image, offscreenPassConstraints.depth.mem, 0));

    VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = fbDepthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = offscreenPassConstraints.depth.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &offscreenPassConstraints.depth.view));

    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

    std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
    // Color attachment
    attchmentDescriptions[0].format = FB_COLOR_FORMAT;
    attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Depth attachment
    attchmentDescriptions[1].format = fbDepthFormat;
    attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
    renderPassInfo.pAttachments = attchmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &offscreenPassConstraints.renderPass));

    VkImageView attachments[2];
    attachments[0] = offscreenPassConstraints.color.view;
    attachments[1] = offscreenPassConstraints.depth.view;

    VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
    fbufCreateInfo.renderPass = offscreenPassConstraints.renderPass;
    fbufCreateInfo.attachmentCount = 2;
    fbufCreateInfo.pAttachments = attachments;
    fbufCreateInfo.width = offscreenPassConstraints.width;
    fbufCreateInfo.height = offscreenPassConstraints.height;
    fbufCreateInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offscreenPassConstraints.frameBuffer));

    // Fill a descriptor for later use in a descriptor set
    offscreenPassConstraints.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    offscreenPassConstraints.descriptor.imageView = offscreenPassConstraints.color.view;
    offscreenPassConstraints.descriptor.sampler = offscreenPassConstraints.sampler;
}



//------------------------------------------OffScreenRenderParticles-----------------------------------

// Setup the offscreen framebuffer for rendering the mirrored scene
    // The color attachment of this framebuffer will then be used to sample from in the fragment shader of the final pass
void GraphModule::prepareOffscreenRenderParticles()
{
    offscreenPassParticles.width =  SIM_WIDTH;
    offscreenPassParticles.height = SIM_HEIGHT;//(float)(FB_DIM_X)*(float)(height/width);

    // Find a suitable depth format
    VkFormat fbDepthFormat;
    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &fbDepthFormat);
    assert(validDepthFormat);

    // Color attachment
    VkImageCreateInfo image = vks::initializers::imageCreateInfo();
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = FB_COLOR_FORMAT;
    image.extent.width =  offscreenPassParticles.width;
    image.extent.height = offscreenPassParticles.height;
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    // We will sample directly from the color attachment
    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT| VK_IMAGE_USAGE_STORAGE_BIT;

    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &offscreenPassParticles.color.image));
    vkGetImageMemoryRequirements(device, offscreenPassParticles.color.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPassParticles.color.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, offscreenPassParticles.color.image, offscreenPassParticles.color.mem, 0));

    VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = FB_COLOR_FORMAT;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = offscreenPassParticles.color.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &colorImageView, nullptr, &offscreenPassParticles.color.view));

    // Create sampler to sample from the attachment in the fragment shader
    VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &offscreenPassParticles.sampler));

    // Depth stencil attachment
    image.format = fbDepthFormat;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &offscreenPassParticles.depth.image));
    vkGetImageMemoryRequirements(device, offscreenPassParticles.depth.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPassParticles.depth.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, offscreenPassParticles.depth.image, offscreenPassParticles.depth.mem, 0));

    VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = fbDepthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = offscreenPassParticles.depth.image;
    VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &offscreenPassParticles.depth.view));

    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

    std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
    // Color attachment
    attchmentDescriptions[0].format = FB_COLOR_FORMAT;
    attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Depth attachment
    attchmentDescriptions[1].format = fbDepthFormat;
    attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
    renderPassInfo.pAttachments = attchmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &offscreenPassParticles.renderPass));

    VkImageView attachments[2];
    attachments[0] = offscreenPassParticles.color.view;
    attachments[1] = offscreenPassParticles.depth.view;

    VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
    fbufCreateInfo.renderPass = offscreenPassParticles.renderPass;
    fbufCreateInfo.attachmentCount = 2;
    fbufCreateInfo.pAttachments = attachments;
    fbufCreateInfo.width = offscreenPassParticles.width;
    fbufCreateInfo.height = offscreenPassParticles.height;
    fbufCreateInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offscreenPassParticles.frameBuffer));

    // Fill a descriptor for later use in a descriptor set
    offscreenPassParticles.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    offscreenPassParticles.descriptor.imageView = offscreenPassParticles.color.view;
    offscreenPassParticles.descriptor.sampler = offscreenPassParticles.sampler;
}
