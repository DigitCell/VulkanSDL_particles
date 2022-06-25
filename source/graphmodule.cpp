#include "graphmodule.hpp"
#include "graphmodule.hpp"


//---------------------------------------------------PREPARE--------------------------------------

void GraphModule::prepare()
{
    //VulkanExampleBase::prepare();

    prepareSimParameters();

    nGraphics.queueFamilyIndex = vulkanDevice->queueFamilyIndices.graphics;
    mapGraphics.queueFamilyIndex = vulkanDevice->queueFamilyIndices.graphics;
    computeBlock.queueFamilyIndex = vulkanDevice->queueFamilyIndices.compute;
    //computeBlock.clearImage.queueFamilyIndex = vulkanDevice->queueFamilyIndices.compute;
    //computeBlock.drawParticles.queueFamilyIndex = vulkanDevice->queueFamilyIndices.compute;

    ndearImgui.Init(
                platform,
                instance,
                physicalDevice,
                device,
                queue,
                cmdPool,
                renderPass,
                descriptorPool,
                pipelineCache,
                swapChain
                );
    loadAssets();

    prepareOffscreen();
    prepareOffscreenRenderConstraints();
    prepareOffscreenRenderParticles();

    prepareComputeBuffers();

    initCreaturesBuffer();
    initParticlesBuffer();
    prepareInstanceData();

    prepareUniformBuffers();

    prepareTextureTarget(&graphics.textures.textureComputeTarget, SIM_WIDTH,SIM_HEIGHT, VK_FORMAT_R8G8B8A8_UNORM, SIM_WIDTH,SIM_HEIGHT);
    prepareTextureTarget(&graphics.textures.textureConstraints,   SIM_WIDTH,SIM_HEIGHT, VK_FORMAT_R8G8B8A8_UNORM, SIM_WIDTH,SIM_HEIGHT);
    prepareTextureTarget(&graphics.textures.computeDrawParticles, SIM_WIDTH,SIM_HEIGHT, VK_FORMAT_R8G8B8A8_UNORM, SIM_WIDTH,SIM_HEIGHT);
    prepareTextureTarget(&graphics.textures.textureColorMap,      SIM_WIDTH,SIM_HEIGHT, VK_FORMAT_R8G8B8A8_UNORM, SIM_WIDTH,SIM_HEIGHT);

   // &graphics.textures.textureConstraints.

    ndearImgui.imageTemp2=ndearImgui.registerTexture(graphics.textures.textureComputeTarget);

    //prepareTextureTarget(&graphics.textures.textureComputeTarget2, graphics.textures.textureColorMap.width, graphics.textures.textureColorMap.height, VK_FORMAT_R8G8B8A8_UNORM);
    ndearImgui.imageTemp=ndearImgui.registerImage(offscreenPass.descriptor.sampler, offscreenPass.descriptor.imageView,offscreenPass.descriptor.imageLayout);
    ndearImgui.imageConstraints=ndearImgui.registerImage(offscreenPassConstraints.descriptor.sampler, offscreenPassConstraints.descriptor.imageView, offscreenPassConstraints.descriptor.imageLayout);

    ndearImgui.imageParticlesRender=ndearImgui.registerImage(offscreenPassParticles.descriptor.sampler, offscreenPassParticles.descriptor.imageView, offscreenPassParticles.descriptor.imageLayout);
    ndearImgui.computeDrawParticles=ndearImgui.registerTexture(graphics.textures.computeDrawParticles);

    setupDescriptorSetLayout();

    prepareParticlePipeline();
    prepareConstraintsPipeline();

    prepareDrawConstraintsPipeline();
    prepareDrawPolylinePipeline();
    prepareMapPipeline();
    prepareDrawTrianglePipeline();

    setupDescriptorPool();
    setupDescriptorSet();

    prepareGraphics();

    prepareComputeUpdateParticles();
    prepareComputeDrawParticles();
    prepareComputeClearImage();

    prepareClearMap();
    prepareUpdateMap();
    prepareNeightbUpdate();
    prepareNeightbSort();
    prepareConstraintsClear();
    prepareConstraintsUpdate();

     updateUniformBuffer(true);


    //ndearImgui.imageTemp=ndearImgui.registerTexture(graphics.textures.textureColorMap);
    //graphics.textures.textureComputeTarget2.image=swapChain.images[0];
    //graphics.textures.textureComputeTarget2.updateDescriptor();

    //buildCommandBuffers();
    prepared = true;
}

void GraphModule::prepareSimParameters()
{
        simParams.mapCellSize=solver.mapCellSize;
        simParams.mapSize=solver.mapSize;

        simDynamicParametrs.tick=0;
        simDynamicParametrs.dt=0.005f;
        simDynamicParametrs.maxVelocity=15.0f;
        simDynamicParametrs.noiseCoeff=0.0f;
        simDynamicParametrs.stiffnessCoeff=0.015;
        simDynamicParametrs.dampingCoeff=0.05;
        simDynamicParametrs.initConstraintDistStart=15.0f;
         simDynamicParametrs.initConstraintDistStop=25.0f;
        simDynamicParametrs.breakConstraintDist=35.0f;
        simDynamicParametrs.radius=1.0;//particleRadius_init;
        //simDynamicParametrs.max_constraints=max_ConstraintLimit;
        //simDynamicParametrs.constraintsNum=1;
        //simDynamicParametrs.rateChangePhase=0.01f;
        simDynamicParametrs.velocityDamping=0.95f;

        simDynamicParametrs.numParticles=simParams.numParticles;
        simDynamicParametrs.max_constraints=CONSTRAINTS_MAX_LIMIT;
}

// Setup and fill the compute shader storage buffers containing the particles
void GraphModule::prepareComputeBuffers()
{
    //Map Buffers;
    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.mapData,
        solver.mapElementsAmount*(MAPCELL_MAX_PARTILCLES+2)*sizeof(int));

    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.mapSize,
        solver.mapElementsAmount*sizeof(int));

    //Neightb Buffers

    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.neightb,
        NUM_PARTICLES_MAX*NEIGHTB_MAX_PARTICLES*sizeof(int));

    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.neightbSort,
        NUM_PARTICLES_MAX*NEIGHTB_MAX_PARTICLES*sizeof(int));

    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.neightbInfo,
        NUM_PARTICLES_MAX*sizeof(float));

    //Constarints buffers

    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.constraintsData,
        CONSTRAINTS_MAX_LIMIT*sizeof(Constraint));

    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.constraintsSize,
        CONSTRAINTS_MAX_LIMIT*sizeof(int));

    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT ,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &computeBlock.buffers.constraintNum,
        2*sizeof(int));


    VkDeviceSize particlesBufferSizeAll = NUM_PARTICLES_MAX * sizeof(Particle);
    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.particlesBuffer,
        particlesBufferSizeAll);
/*
    vulkanDevice->createBuffer(
        // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &computeBlock.buffers.creatureData,
        NUM_CREATURES_MAX*sizeof (Creature));
*/
}

// Setup and fill the compute shader storage buffers containing the particles
void GraphModule::initCreaturesBuffer()
{
    //auto creaturesList=solver.CreatureListInit(NUM_CREATURES_INIT);
   // simDynamicParametrs.numCreatures=creaturesList.size();
/*
    VkDeviceSize creaturesListSize = creaturesList.size() * sizeof(Creature);


    vks::Buffer stagingBuffer;

    vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        creaturesListSize,
        creaturesList.data());

    // Copy from staging buffer to storage buffer
    VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy copyRegion = {0,0,creaturesListSize};
    copyRegion.size = creaturesListSize;
    vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, computeBlock.buffers.creatureData.buffer, 1, &copyRegion);
    // Execute a transfer barrier to the compute queue, if necessary
    if (nGraphics.queueFamilyIndex != computeBlock.queueFamilyIndex)
    {
        VkBufferMemoryBarrier buffer_barrier =
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            0,
            nGraphics.queueFamilyIndex,
            computeBlock.queueFamilyIndex,
            computeBlock.buffers.creatureData.buffer,
            0,
            creaturesListSize //computeBlock.buffers.creatureData.size
        };

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            1, &buffer_barrier,
            0, nullptr);
    }
    vulkanDevice->flushCommandBuffer(copyCmd, queue, true);
    stagingBuffer.destroy();
*/
}


// Setup and fill the compute shader storage buffers containing the particles
void GraphModule::initParticlesBuffer()
{
    VK_CHECK_RESULT(computeBlock.buffers.constraintNum.map());

    //auto particleBuffer=solver.ParticleRadiusInit(NUM_PARTICLES_INIT, 0);
    auto particleBuffer=solver.ParticleInit(NUM_PARTICLES_INIT);
    VkDeviceSize particlesBufferSize = particleBuffer.size() * sizeof(Particle);
    simParams.numParticles=particleBuffer.size();
    simDynamicParametrs.numParticles=simParams.numParticles;

    vks::Buffer stagingBuffer;

    vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        particlesBufferSize,
        particleBuffer.data());

    // Copy from staging buffer to storage buffer
    VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy copyRegion = {0,0,particlesBufferSize };
    copyRegion.size = particlesBufferSize;
    vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, computeBlock.buffers.particlesBuffer.buffer, 1, &copyRegion);
    // Execute a transfer barrier to the compute queue, if necessary
    if (nGraphics.queueFamilyIndex != computeBlock.queueFamilyIndex)
    {
        VkBufferMemoryBarrier buffer_barrier =
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            0,
            nGraphics.queueFamilyIndex,
            computeBlock.queueFamilyIndex,
            computeBlock.buffers.particlesBuffer.buffer,
            0,
            computeBlock.buffers.particlesBuffer.size
        };

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            1, &buffer_barrier,
            0, nullptr);
    }
    vulkanDevice->flushCommandBuffer(copyCmd, queue, true);
    stagingBuffer.destroy();

}

void GraphModule::addParticlesToBuffer(int num_Particles)
{
    simCommandParams.addParticles=false;
    auto particleBuffer=solver.ParticleInit(num_Particles);
    VkDeviceSize particlesBufferSize = particleBuffer.size() * sizeof(Particle);

    vks::Buffer stagingBuffer;

    vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        particlesBufferSize,
        particleBuffer.data());


    // Copy from staging buffer to storage buffer
    VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy copyRegion = {0,simParams.numParticles*sizeof(Particle),particlesBufferSize };
    copyRegion.size = particlesBufferSize;
    vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, computeBlock.buffers.particlesBuffer.buffer, 1, &copyRegion);

    simParams.numParticles+=num_Particles;
    simDynamicParametrs.numParticles=simParams.numParticles;

    // Execute a transfer barrier to the compute queue, if necessary
    if (nGraphics.queueFamilyIndex != computeBlock.queueFamilyIndex)
    {
        VkBufferMemoryBarrier buffer_barrier =
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            0,
            nGraphics.queueFamilyIndex,
            computeBlock.queueFamilyIndex,
            computeBlock.buffers.particlesBuffer.buffer,
            0,
            computeBlock.buffers.particlesBuffer.size
        };

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            1, &buffer_barrier,
            0, nullptr);
    }
    vulkanDevice->flushCommandBuffer(copyCmd, queue, true);

    buildComputeUpdateParticlesCommandBuffer();
    buildUpdateMapCommandBuffer();
    buildComputeDrawParticlesCommandBuffer();
    buildNeightbUpdateCommandBuffer();
    buildNeightbSortCommandBuffer();

     stagingBuffer.destroy();
}

void GraphModule::resetSim(int num_Particles)
{
    simCommandParams.resetSim=false;
    auto particleBuffer=solver.ParticleInit(num_Particles);
    VkDeviceSize particlesBufferSize = particleBuffer.size() * sizeof(Particle);

    vks::Buffer stagingBuffer;

    vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        particlesBufferSize,
        particleBuffer.data());

    // Copy from staging buffer to storage buffer
    VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy copyRegion = {};
    copyRegion.size = particlesBufferSize;
    vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, computeBlock.buffers.particlesBuffer.buffer, 1, &copyRegion);

    simParams.numParticles=particleBuffer.size();
    simDynamicParametrs.numParticles=simParams.numParticles;

    // Execute a transfer barrier to the compute queue, if necessary
    if (nGraphics.queueFamilyIndex != computeBlock.queueFamilyIndex)
    {
        VkBufferMemoryBarrier buffer_barrier =
        {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            0,
            nGraphics.queueFamilyIndex,
            computeBlock.queueFamilyIndex,
            computeBlock.buffers.particlesBuffer.buffer,
            0,
            computeBlock.buffers.particlesBuffer.size
        };

        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            1, &buffer_barrier,
            0, nullptr);
    }
    vulkanDevice->flushCommandBuffer(copyCmd, queue, true);

    buildComputeUpdateParticlesCommandBuffer();
    buildUpdateMapCommandBuffer();
    buildComputeDrawParticlesCommandBuffer();
    buildNeightbUpdateCommandBuffer();
    buildNeightbSortCommandBuffer();

    stagingBuffer.destroy();

}


void GraphModule::prepareInstanceData()
{
    //std::vector<InstanceData> instanceData=solver.ParticleInstanceInit(NUM_PARTICLES_INIT, nGraphics.textures.rocks.layerCount);

   // VkDeviceSize particlesBufferSize =  instanceData.size() * sizeof(InstanceData);
    VkDeviceSize particlesBufferSizeAll = NUM_PARTICLES_MAX * sizeof(InstanceData);
    nGraphics.instanceBuffer.size =       NUM_PARTICLES_MAX * sizeof(InstanceData);

    {
        vulkanDevice->createBuffer(
            // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &nGraphics.instanceBuffer,
            particlesBufferSizeAll);
    }
    {
        vulkanDevice->createBuffer(
            // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &nGraphics.instanceConstraints,
            CONSTRAINTS_MAX_LIMIT*sizeof(glm::vec2));
    }

    {
        vulkanDevice->createBuffer(
            // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &nGraphics.instancePolygons,
            CONSTRAINTS_MAX_LIMIT*sizeof(glm::vec2));
    }

    {
        vulkanDevice->createBuffer(
            // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &nGraphics.drawPolyline,
            NUM_PARTICLES_MAX*sizeof(glm::vec2));
    }

    {
        vulkanDevice->createBuffer(
            // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &nGraphics.drawTriangle,
            NUM_PARTICLES_MAX*sizeof(TrianglesDrawStruct));
    }
/*
 {
        vector<glm::vec2> tempList;

        for(auto particle1:solver.particleBuffer)
        {
            for(auto particle2:solver.particleBuffer)
            {
                tempList.push_back(glm::vec2(particle1.pos.x,particle1.pos.y));
                tempList.push_back(glm::vec2(particle2.pos.x,particle2.pos.y));
            }
        }

        simDynamicParametrs.constraintsNum=tempList.size();


        VkDeviceSize particlesBufferSize =  tempList.size() * sizeof(glm::vec2);
        vks::Buffer stagingBuffer;

        vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer,
            particlesBufferSize,
            tempList.data());

        vulkanDevice->createBuffer(
            // The SSBO will be used as a storage buffer for the compute pipeline and as a vertex buffer in the graphics pipeline
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &nGraphics.instanceConstraints,
            particlesBufferSize);

        // Copy from staging buffer to storage buffer
        VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        VkBufferCopy copyRegion = {};
        copyRegion.size = particlesBufferSize;
        vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, nGraphics.instanceConstraints.buffer, 1, &copyRegion);
        // Execute a transfer barrier to the compute queue, if necessary
        vulkanDevice->flushCommandBuffer(copyCmd, queue, true);
        stagingBuffer.destroy();


    }
*/
}

void GraphModule::prepareUniformBuffers()
{
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &nGraphics.uniformBuffers2.scene,
        sizeof(nGraphics.uboVS)));
    // Map persistent
    VK_CHECK_RESULT(nGraphics.uniformBuffers2.scene.map());


    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &nGraphics.uniformBuffers2.constraint_scene,
        sizeof(nGraphics.constraintUBO)));
    // Map persistent
    VK_CHECK_RESULT(nGraphics.uniformBuffers2.constraint_scene.map());

    vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &computeBlock.uniforms.simParamsData,
        1*sizeof(SimParametrs));
    //Map buffer to host
    VK_CHECK_RESULT(computeBlock.uniforms.simParamsData.map());

    vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &computeBlock.uniforms.simDynamicParamsData,
        1*sizeof(SimDynamicParametrs));
    //Map buffer to host
    VK_CHECK_RESULT(computeBlock.uniforms.simDynamicParamsData.map());


}

void GraphModule::updateUniformBuffer(bool viewChanged)
{

    if(simCommandParams.resetSim)
    {
        resetSim(NUM_PARTICLES_INIT);
        simDynamicParametrs.tick=0;
    }

    if(simCommandParams.addParticles)
    {
        addParticlesToBuffer(NUM_PARTICLES_ADD);
    }

    if(simCommandParams.readPolygons)
    {
        ReadPolygons();
    }

    if(simCommandParams.offsetPolygons)
    {
        OffsetPolygons();
    }

    if(simCommandParams.triangulate)
    {
        TriangulatePolygons();
    }

    if (viewChanged)
    {
        nGraphics.uboVS.projection = camera.matrices.perspective;
        nGraphics.uboVS.view = camera.matrices.view;
    }

    if(simCommandParams.saveImage)
    {
        saveScreenshot("screenshot.ppm");
        simCommandParams.saveImage=false;
    }



    nGraphics.uboVS.worldSize=glm::vec2(SIM_WIDTH, SIM_HEIGHT);
    nGraphics.uboVS.mapSize=glm::vec2(MAP_SIZE_X, MAP_SIZE_Y);
    //simParams.mapCellSize=solver.mapCellSize;
    //if(simCommandParams.runLoop)
    {
      //  nGraphics.uboVS.locSpeed  += frameTimer * 0.00035f;
     //   nGraphics.uboVS.globSpeed += frameTimer * 0.01f;
    }
    memcpy(nGraphics.uniformBuffers2.scene.mapped, &nGraphics.uboVS, sizeof(nGraphics.uboVS));


    nGraphics.constraintUBO.worldSize=glm::vec2(SIM_WIDTH, SIM_HEIGHT);
    nGraphics.constraintUBO.mapSize=  glm::vec2(MAP_SIZE_X, MAP_SIZE_Y);
    memcpy(nGraphics.uniformBuffers2.constraint_scene.mapped, &nGraphics.constraintUBO, sizeof(nGraphics.constraintUBO));

    graphics.textures.textureConstraints.descriptor.sampler=offscreenPassConstraints.descriptor.sampler;
/*
    computeBlock.ubo.deltaT = frameTimer * 2.5f;
    if (true)//!attachToCursor)
    {
        computeBlock.ubo.destX = sin(glm::radians(timer * 360.0f)) * 0.75f;
        computeBlock.ubo.destY+= 0.0005;
    }
    else
    {
        float normalizedMx = (mousePos.x - static_cast<float>(width / 2)) / static_cast<float>(width / 2);
        float normalizedMy = (mousePos.y - static_cast<float>(height / 2)) / static_cast<float>(height / 2);
        computeBlock.ubo.destX = normalizedMx;
        computeBlock.ubo.destY = normalizedMy;
    }
*/

    simParams.mapCellSize.x=solver.mapCellSize.x;
    simParams.mapCellSize.y=solver.mapCellSize.y;

    simDynamicParametrs.tick++;

    //computeBlock.buffers.constraintNum.copyTo(&simDynamicParametrs.constraintsNum, 1*sizeof(int));
    simDynamicParametrs.constraintsNum=0;
    simDynamicParametrs.polygonsNum=0;
    vector<int> copyData {simDynamicParametrs.constraintsNum,simDynamicParametrs.polygonsNum};
   // memcpy(computeBlock.buffers.constraintNum.mapped, &simDynamicParametrs.constraintsNum,sizeof(int));
    memcpy(copyData.data(), computeBlock.buffers.constraintNum.mapped, 2*sizeof(int));

    memcpy(computeBlock.uniforms.simParamsData.mapped, &simParams, sizeof(SimParametrs));
    memcpy(computeBlock.uniforms.simDynamicParamsData.mapped, &simDynamicParametrs, sizeof(SimDynamicParametrs));

    simDynamicParametrs.constraintsNum=copyData[0];
    simDynamicParametrs.polygonsNum=copyData[1];



}

void GraphModule::writeUniformBuffer()
{
    vector<int> copyData {simDynamicParametrs.constraintsNum,simDynamicParametrs.polygonsNum};
    memcpy(copyData.data(), computeBlock.buffers.constraintNum.mapped, sizeof(int));
   // cout<<simDynamicParametrs.constraintsNum<<endl;
}


void GraphModule::ReadPolygons()
{

    simCommandParams.readPolygons=false;

    nGraphics.instancePolygons.map(2*simDynamicParametrs.polygonsNum*sizeof(glm::vec2));

    vector<glm::vec2> points;
    points.resize(simDynamicParametrs.polygonsNum*2);
    memcpy(points.data(), nGraphics.instancePolygons.mapped, 2*simDynamicParametrs.polygonsNum*sizeof(glm::vec2));
    nGraphics.instancePolygons.unmap();


}


void GraphModule::OffsetPolygons()
{


}

void GraphModule::TriangulatePolygons()
{

    simCommandParams.triangulate=false;

}

//-----------------------------------------DRAW-----------------------------------

void GraphModule::draw()
{
    if(simCommandParams.runLoop or simDynamicParametrs.tick<3)
    {

        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.clearImage.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &graphics.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.clearImage.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }

        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.clearMap.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.clearImage.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.clearMap.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }

        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.updateMap.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.clearMap.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.updateMap.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }

        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.neightbUpdate.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.updateMap.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.neightbUpdate.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }

        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.neightbSort.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.neightbUpdate.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.neightbSort.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }

        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.constraintsClear.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.neightbSort.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.constraintsClear.semaphore;//&computeBlock.updateParticles.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }

        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.constraintsUpdate.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.constraintsClear.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.constraintsUpdate.semaphore;//&computeBlock.updateParticles.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }


        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.updateParticles.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.constraintsUpdate.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.updateParticles.semaphore;//&computeBlock.updateParticles.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }



        {
            // Wait for rendering finished
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            // Submit compute commands
            VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
            computeSubmitInfo.commandBufferCount = 1;
            computeSubmitInfo.pCommandBuffers = &computeBlock.drawParticles.commandBuffer;
            computeSubmitInfo.waitSemaphoreCount = 1;
            computeSubmitInfo.pWaitSemaphores = &computeBlock.updateParticles.semaphore;
            computeSubmitInfo.pWaitDstStageMask = &waitStageMask;
            computeSubmitInfo.signalSemaphoreCount = 1;
            computeSubmitInfo.pSignalSemaphores = &computeBlock.drawParticles.semaphore;;//&computeBlock.drawParticles.semaphore;
            VK_CHECK_RESULT(vkQueueSubmit(computeBlock.queue, 1, &computeSubmitInfo, VK_NULL_HANDLE));
        }

    }

      writeUniformBuffer();

    nVulkanBase::prepareFrame();

    if(simCommandParams.runLoop or simDynamicParametrs.tick<3)
    {
        VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore graphicsWaitSemaphores[] =   {  computeBlock.drawParticles.semaphore, semaphores.presentComplete };
        VkSemaphore graphicsSignalSemaphores[] = { graphics.semaphore, semaphores.renderComplete };

        // Command buffer to be sumitted to the queue
        // Submit graphics commands
        buildCommandBuffer(currentBuffer);
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
        submitInfo.waitSemaphoreCount = 2;
        submitInfo.pWaitSemaphores = graphicsWaitSemaphores;
        submitInfo.pWaitDstStageMask = graphicsWaitStageMasks;
        submitInfo.signalSemaphoreCount = 2;
        submitInfo.pSignalSemaphores = graphicsSignalSemaphores;
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
        // Submit to queue
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    }
    else
    {
        VkPipelineStageFlags graphicsWaitStageMasks[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore graphicsWaitSemaphores[] =   { };
        VkSemaphore graphicsSignalSemaphores[] = { graphics.semaphore, semaphores.renderComplete };

        // Command buffer to be sumitted to the queue
        // Submit graphics commands
        buildCommandBuffer(currentBuffer);
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = graphicsWaitSemaphores;
        submitInfo.pWaitDstStageMask = graphicsWaitStageMasks;
        submitInfo.signalSemaphoreCount = 2;
        submitInfo.pSignalSemaphores = graphicsSignalSemaphores;
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
        // Submit to queue
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    }
    nVulkanBase::submitFrame();
}
