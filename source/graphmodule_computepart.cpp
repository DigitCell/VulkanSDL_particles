#include "graphmodule.hpp"


//------------------------------------------Compute drawParticles-------------------------------------------


void GraphModule::prepareComputeDrawParticles()
{
    // Get a compute queue from the device
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);
    // Create compute pipeline
    // Compute pipelines are created separate from graphics pipelines even if they use the same queue

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0: Input image (read-only)
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        // Binding 1: Output image (write)
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT,
            1),

        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            3),

         // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            4),

    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(
                setLayoutBindings.data(),
                static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr, &computeBlock.drawParticles.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
                &computeBlock.drawParticles.descriptorSetLayout, 1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &computeBlock.drawParticles.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo
            (descriptorPool,
             &computeBlock.drawParticles.descriptorSetLayout,
             1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.drawParticles.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
        vks::initializers::writeDescriptorSet(
        computeBlock.drawParticles.descriptorSet,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        0,
        &graphics.textures.textureColorMap.descriptor),

        vks::initializers::writeDescriptorSet(
        computeBlock.drawParticles.descriptorSet,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        1,
        &graphics.textures.textureComputeTarget.descriptor),

        // particle position storage buffer
        vks::initializers::writeDescriptorSet(
        computeBlock.drawParticles.descriptorSet,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        2,
        &nGraphics.instanceBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
        computeBlock.drawParticles.descriptorSet,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        3,
        &computeBlock.buffers.particlesBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
        computeBlock.drawParticles.descriptorSet,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        4,
        &computeBlock.uniforms.simDynamicParamsData.descriptor)


    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create compute shader pipelines
    VkComputePipelineCreateInfo computePipelineCreateInfo =
        vks::initializers::computePipelineCreateInfo(computeBlock.drawParticles.pipelineLayout, 0);

    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/particleImage.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.drawParticles.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = vulkanDevice->queueFamilyIndices.compute;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    VkCommandBufferAllocateInfo cmdBufAllocateInfo =vks::initializers::commandBufferAllocateInfo( computeBlock.commandPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY,1);
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &computeBlock.drawParticles.commandBuffer));

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.drawParticles.semaphore));

    // Build a single command buffer containing the compute dispatch commands
    buildComputeDrawParticlesCommandBuffer();
}

void GraphModule::buildComputeDrawParticlesCommandBuffer()
{
    // Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
    vkQueueWaitIdle(computeBlock.queue);

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.drawParticles.commandBuffer, &cmdBufInfo));

    vkCmdBindPipeline(computeBlock.drawParticles.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.drawParticles.pipeline);
    vkCmdBindDescriptorSets(computeBlock.drawParticles.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.drawParticles.pipelineLayout, 0, 1, &computeBlock.drawParticles.descriptorSet, 0, 0);

    int groupcount = ((simParams.numParticles) / 256) + 1;
    vkCmdDispatch(computeBlock.drawParticles.commandBuffer,  groupcount, 1, 1);
   // vkCmdDispatch(computeBlock.drawParticles.commandBuffer, graphics.textures.textureComputeTarget.width / 16, graphics.textures.textureComputeTarget.height / 16, 1);

    vkEndCommandBuffer(computeBlock.drawParticles.commandBuffer);
}



//------------------------------------------------------Compute updateParticles-----------------------------------
void GraphModule::prepareComputeUpdateParticles()
{
    // Create a compute capable device queue
    // The VulkanDevice::createLogicalDevice functions finds a compute capable queue and prefers queue families that only support compute
    // Depending on the implementation this may result in different queue family indices for graphics and computes,
    // requiring proper synchronization (see the memory and pipeline barriers)
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);

    // Create compute pipeline
    // Compute pipelines are created separate from graphics pipelines even if they use the same queue (family index)

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Particle position storage buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            3),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            4),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            5),

        // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            6),

        // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            8),

        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            9),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.updateParticles.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.updateParticles.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.updateParticles.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.updateParticles.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.updateParticles.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets =
    {
        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            0,
            &computeBlock.buffers.particlesBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            &nGraphics.instanceBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            2,
            &computeBlock.buffers.neightb.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            3,
            &computeBlock.buffers.neightbSort.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            4,
            &computeBlock.buffers.neightbInfo.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            5,
            &computeBlock.buffers.constraintsData.descriptor),

          //  Uniform buffer
        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            6,
            &computeBlock.uniforms.simParamsData.descriptor),


        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            8,
            &computeBlock.uniforms.simDynamicParamsData.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateParticles.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            9,
            &computeBlock.buffers.constraintNum.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create pipeline
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(computeBlock.updateParticles.pipelineLayout, 0);
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/particle.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.updateParticles.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    computeBlock.updateParticles.commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool);

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.updateParticles.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeBlock.updateParticles.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    // Build a single command buffer containing the compute dispatch commands
    buildComputeUpdateParticlesCommandBuffer();

    // If graphics and compute queue family indices differ, acquire and immediately release the storage buffer, so that the initial acquire from the graphics command buffers are matched up properly
    if (nGraphics.queueFamilyIndex != computeBlock.queueFamilyIndex)
    {
        // Create a transient command buffer for setting up the initial buffer transfer state
        VkCommandBuffer transferCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool, true);

        VkBufferMemoryBarrier acquire_buffer_barrier =
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_SHADER_WRITE_BIT,
            nGraphics.queueFamilyIndex,
            computeBlock.queueFamilyIndex,
            computeBlock.buffers.particlesBuffer.buffer,
            0,
            computeBlock.buffers.particlesBuffer.size
        };
        vkCmdPipelineBarrier(
            transferCmd,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            1, &acquire_buffer_barrier,
            0, nullptr);

        VkBufferMemoryBarrier release_buffer_barrier =
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_SHADER_WRITE_BIT,
            0,
            computeBlock.queueFamilyIndex,
            nGraphics.queueFamilyIndex,
            computeBlock.buffers.particlesBuffer.buffer,
            0,
            computeBlock.buffers.particlesBuffer.size
        };
        vkCmdPipelineBarrier(
            transferCmd,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            0, nullptr,
            1, &release_buffer_barrier,
            0, nullptr);

        vulkanDevice->flushCommandBuffer(transferCmd, computeBlock.queue, computeBlock.commandPool);
    }
}


void GraphModule::buildComputeUpdateParticlesCommandBuffer()
{
    // Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.updateParticles.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.updateParticles.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.updateParticles.pipeline);
    vkCmdBindDescriptorSets(computeBlock.updateParticles.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.updateParticles.pipelineLayout, 0, 1, &computeBlock.updateParticles.descriptorSet, 0, 0);

    //vkCmdBindVertexBuffers(computeBlock.updateParticles.commandBuffer, INSTANCE_BUFFER_BIND_ID, 1, &nGraphics.instanceBuffer.buffer, 0);
    int groupcount = ((simParams.numParticles) / 256) + 1;
    vkCmdDispatch(computeBlock.updateParticles.commandBuffer, groupcount , 1, 1);
    // Add barrier to ensure that compute shader has finished writing to the buffer
    // Without this the (rendering) vertex shader may display incomplete results (partial data from last frame)
    vkEndCommandBuffer(computeBlock.updateParticles.commandBuffer);

}


//------------------------------------------Compute clearImage-------------------------------------------


void GraphModule::prepareComputeClearImage()
{
    // Get a compute queue from the device
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);

    // Create compute pipeline
    // Compute pipelines are created separate from graphics pipelines even if they use the same queue

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0: Input image (read-only)
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        // Binding 1: Output image (write)
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT,
            1),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.clearImage.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.clearImage.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.clearImage.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.clearImage.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.clearImage.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
       vks::initializers::writeDescriptorSet(
       computeBlock.clearImage.descriptorSet,
       VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
       0,
       &graphics.textures.textureComputeTarget.descriptor),

       vks::initializers::writeDescriptorSet(
       computeBlock.clearImage.descriptorSet,
       VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
       1,
       &graphics.textures.textureColorMap.descriptor),

       vks::initializers::writeDescriptorSet(
       computeBlock.clearImage.descriptorSet,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       2,
       &computeBlock.uniforms.simDynamicParamsData.descriptor),
   };
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create compute shader pipelines
    VkComputePipelineCreateInfo computePipelineCreateInfo =
        vks::initializers::computePipelineCreateInfo(computeBlock.clearImage.pipelineLayout, 0);

    // One pipeline for each effect
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/clearImage.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.clearImage.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex =  computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vks::initializers::commandBufferAllocateInfo(
            computeBlock.commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1);

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &computeBlock.clearImage.commandBuffer));

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.clearImage.semaphore));

    // Build a single command buffer containing the compute dispatch commands
    buildComputeClearImageCommandBuffer();

}

void GraphModule::buildComputeClearImageCommandBuffer()
{
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.clearImage.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.clearImage.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.clearImage.pipeline);
    vkCmdBindDescriptorSets(computeBlock.clearImage.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.clearImage.pipelineLayout, 0, 1, &computeBlock.clearImage.descriptorSet, 0, 0);
    vkCmdDispatch(computeBlock.clearImage.commandBuffer, SIM_WIDTH / 16, SIM_HEIGHT / 16, 1);
    vkEndCommandBuffer(computeBlock.clearImage.commandBuffer);
}


//------------------------------------------------------Compute ClearMap-----------------------------------
void GraphModule::prepareClearMap()
{
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Particle position storage buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.clearMap.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.clearMap.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.clearMap.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.clearMap.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.clearMap.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets =
    {
        // particle position storage buffer
        vks::initializers::writeDescriptorSet(
            computeBlock.clearMap.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            0,
            &computeBlock.buffers.mapSize.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create pipeline
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(computeBlock.clearMap.pipelineLayout, 0);
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/MapClear.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.clearMap.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    computeBlock.clearMap.commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool);

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.clearMap.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeBlock.clearMap.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    // Build a single command buffer containing the compute dispatch commands
    buildClearMapCommandBuffer();
}


void GraphModule::buildClearMapCommandBuffer()
{
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.clearMap.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.clearMap.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.clearMap.pipeline);
    vkCmdBindDescriptorSets(computeBlock.clearMap.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.clearMap.pipelineLayout, 0, 1, &computeBlock.clearMap.descriptorSet, 0, 0);
    int groupcount = ((solver.mapElementsAmount) / 256) + 1;
    vkCmdDispatch(computeBlock.clearMap.commandBuffer,  groupcount , 1, 1);
    vkEndCommandBuffer(computeBlock.clearMap.commandBuffer);
}



//------------------------------------------------------Compute UpdateMap-----------------------------------
void GraphModule::prepareUpdateMap()
{
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Particle position storage buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
        // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            3),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.updateMap.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.updateMap.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.updateMap.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.updateMap.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.updateMap.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets =
    {
        vks::initializers::writeDescriptorSet(
            computeBlock.updateMap.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            0,
            &computeBlock.buffers.particlesBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateMap.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            &computeBlock.buffers.mapSize.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.updateMap.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            2,
            &computeBlock.buffers.mapData.descriptor),
        vks::initializers::writeDescriptorSet(
            computeBlock.updateMap.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            3,
            &computeBlock.uniforms.simParamsData.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create pipeline
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(computeBlock.updateMap.pipelineLayout, 0);
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/MapUpdate.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.updateMap.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    computeBlock.updateMap.commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool);

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.updateMap.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeBlock.updateMap.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    // Build a single command buffer containing the compute dispatch commands
    buildUpdateMapCommandBuffer();

}


void GraphModule::buildUpdateMapCommandBuffer()
{
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.updateMap.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.updateMap.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.updateMap.pipeline);
    vkCmdBindDescriptorSets(computeBlock.updateMap.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.updateMap.pipelineLayout, 0, 1, &computeBlock.updateMap.descriptorSet, 0, 0);
    int groupcount = ((simParams.numParticles) / 256) + 1;
    vkCmdDispatch(computeBlock.updateMap.commandBuffer,  groupcount, 1, 1);
    vkEndCommandBuffer(computeBlock.updateMap.commandBuffer);
}



//------------------------------------------------------Compute NeightbUpdate-----------------------------------
void GraphModule::prepareNeightbUpdate()
{
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Particle position storage buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            3),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            4),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            5),

        // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            6),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.neightbUpdate.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.neightbUpdate.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.neightbUpdate.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.neightbUpdate.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.neightbUpdate.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets =
    {
        vks::initializers::writeDescriptorSet(
            computeBlock.neightbUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            0,
            &computeBlock.buffers.particlesBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            &computeBlock.buffers.mapSize.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            2,
            &computeBlock.buffers.mapData.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            3,
            &computeBlock.buffers.neightb.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            4,
            &computeBlock.buffers.neightbSort.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            5,
            &computeBlock.buffers.neightbInfo.descriptor),

        //uniform
        vks::initializers::writeDescriptorSet(
            computeBlock.neightbUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            6,
            &computeBlock.uniforms.simParamsData.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(computeBlock.neightbUpdate.pipelineLayout, 0);
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/NeightUpdate.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.neightbUpdate.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    computeBlock.neightbUpdate.commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool);

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.neightbUpdate.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeBlock.neightbUpdate.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    // Build a single command buffer containing the compute dispatch commands
    buildNeightbUpdateCommandBuffer();

}


void GraphModule::buildNeightbUpdateCommandBuffer()
{
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.neightbUpdate.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.neightbUpdate.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.neightbUpdate.pipeline);
    vkCmdBindDescriptorSets(computeBlock.neightbUpdate.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.neightbUpdate.pipelineLayout, 0, 1, &computeBlock.neightbUpdate.descriptorSet, 0, 0);
    int groupcount = ((simParams.numParticles) / 256) + 1;
    vkCmdDispatch(computeBlock.neightbUpdate.commandBuffer,  groupcount, 1, 1);
    vkEndCommandBuffer(computeBlock.neightbUpdate.commandBuffer);
}


//------------------------------------------------------Compute NeightbSort-----------------------------------
void GraphModule::prepareNeightbSort()
{
    // Create a compute capable device queue
    // The VulkanDevice::createLogicalDevice functions finds a compute capable queue and prefers queue families that only support compute
    // Depending on the implementation this may result in different queue family indices for graphics and computes,
    // requiring proper synchronization (see the memory and pipeline barriers)
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);

    // Create compute pipeline
    // Compute pipelines are created separate from graphics pipelines even if they use the same queue (family index)

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Particle position storage buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            3),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            4),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            5),

        // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            6),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.neightbSort.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.neightbSort.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.neightbSort.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.neightbSort.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.neightbSort.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets =
    {
        vks::initializers::writeDescriptorSet(
            computeBlock.neightbSort.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            0,
            &computeBlock.buffers.particlesBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbSort.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            &computeBlock.buffers.mapSize.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbSort.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            2,
            &computeBlock.buffers.mapData.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbSort.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            3,
            &computeBlock.buffers.neightb.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbSort.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            4,
            &computeBlock.buffers.neightbSort.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.neightbSort.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            5,
            &computeBlock.buffers.neightbInfo.descriptor),

        //uniforms
        vks::initializers::writeDescriptorSet(
            computeBlock.neightbSort.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            6,
            &computeBlock.uniforms.simParamsData.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create pipeline
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(computeBlock.neightbSort.pipelineLayout, 0);
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/NeightSort.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.neightbSort.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    computeBlock.neightbSort.commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool);

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.neightbSort.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeBlock.neightbSort.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));
    // Build a single command buffer containing the compute dispatch commands
    buildNeightbSortCommandBuffer();

}


void GraphModule::buildNeightbSortCommandBuffer()
{
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.neightbSort.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.neightbSort.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.neightbSort.pipeline);
    vkCmdBindDescriptorSets(computeBlock.neightbSort.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.neightbSort.pipelineLayout, 0, 1, &computeBlock.neightbSort.descriptorSet, 0, 0);
    int groupcount = ((simParams.numParticles) / 256) + 1;
    vkCmdDispatch(computeBlock.neightbSort.commandBuffer,  groupcount, 1, 1);
    vkEndCommandBuffer(computeBlock.neightbSort.commandBuffer);
}



//------------------------------------------------------Compute ConstraintsClear-----------------------------------
void GraphModule::prepareConstraintsClear()
{
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Particle position storage buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            3),

        // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            4),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.constraintsClear.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.constraintsClear.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.constraintsClear.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.constraintsClear.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.constraintsClear.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets =
    {
        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsClear.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            0,
            &computeBlock.buffers.particlesBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsClear.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            &computeBlock.buffers.constraintsData.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsClear.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            2,
            &nGraphics.instanceConstraints.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsClear.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            3,
            &computeBlock.buffers.constraintNum.descriptor),

        //uniform
        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsClear.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            4,
            &computeBlock.uniforms.simDynamicParamsData.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create pipeline
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(computeBlock.constraintsClear.pipelineLayout, 0);
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/ConstraintsClear.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.constraintsClear.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    computeBlock.constraintsClear.commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool);

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.constraintsClear.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeBlock.constraintsClear.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    // Build a single command buffer containing the compute dispatch commands
    buildConstraintsClearCommandBuffer();

}


void GraphModule::buildConstraintsClearCommandBuffer()
{
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.constraintsClear.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.constraintsClear.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.constraintsClear.pipeline);
    vkCmdBindDescriptorSets(computeBlock.constraintsClear.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.constraintsClear.pipelineLayout, 0, 1, &computeBlock.constraintsClear.descriptorSet, 0, 0);
    int groupcount = (CONSTRAINTS_MAX_LIMIT/ 256) + 1;
    vkCmdDispatch(computeBlock.constraintsClear.commandBuffer,  groupcount , 1, 1);
    vkEndCommandBuffer(computeBlock.constraintsClear.commandBuffer);
}




//------------------------------------------------------Compute ConstraintsUpdate-----------------------------------
void GraphModule::prepareConstraintsUpdate()
{
    vkGetDeviceQueue(device, computeBlock.queueFamilyIndex, 0, &computeBlock.queue);

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        // Binding 0 : Particle position storage buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            0),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            1),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            2),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            3),
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            4),

        // Binding 1 : Uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT,
            5),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr,	&computeBlock.constraintsUpdate.descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &computeBlock.constraintsUpdate.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr,	&computeBlock.constraintsUpdate.pipelineLayout));

    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            descriptorPool,
            &computeBlock.constraintsUpdate.descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &computeBlock.constraintsUpdate.descriptorSet));

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets =
    {

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            0,
            &computeBlock.buffers.particlesBuffer.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            &computeBlock.buffers.constraintsData.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            2,
            &nGraphics.instanceConstraints.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            3,
            &nGraphics.instancePolygons.descriptor),

        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            4,
            &computeBlock.buffers.constraintNum.descriptor),

        //uniform
        vks::initializers::writeDescriptorSet(
            computeBlock.constraintsUpdate.descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            5,
            &computeBlock.uniforms.simDynamicParamsData.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);

    // Create pipeline
    VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(computeBlock.constraintsUpdate.pipelineLayout, 0);
    computePipelineCreateInfo.stage = loadShader("../data/shaders/compute/ConstraintsUpdate.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &computeBlock.constraintsUpdate.pipeline));

    // Separate command pool as queue family for compute may be different than graphics
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = computeBlock.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &computeBlock.commandPool));

    // Create a command buffer for compute operations
    computeBlock.constraintsUpdate.commandBuffer = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, computeBlock.commandPool);

    // Semaphore for compute & graphics sync
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &computeBlock.constraintsUpdate.semaphore));

    // Signal the semaphore
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeBlock.constraintsUpdate.semaphore;
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    // Build a single command buffer containing the compute dispatch commands
    buildConstraintsUpdateCommandBuffer();

}


void GraphModule::buildConstraintsUpdateCommandBuffer()
{
    vkQueueWaitIdle(computeBlock.queue);
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(computeBlock.constraintsUpdate.commandBuffer, &cmdBufInfo));
    vkCmdBindPipeline(computeBlock.constraintsUpdate.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.constraintsUpdate.pipeline);
    vkCmdBindDescriptorSets(computeBlock.constraintsUpdate.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeBlock.constraintsUpdate.pipelineLayout, 0, 1, &computeBlock.constraintsUpdate.descriptorSet, 0, 0);
    int groupcount = (CONSTRAINTS_MAX_LIMIT/ 256) + 1;
    vkCmdDispatch(computeBlock.constraintsUpdate.commandBuffer,  groupcount, 1, 1);
    vkEndCommandBuffer(computeBlock.constraintsUpdate.commandBuffer);
}
