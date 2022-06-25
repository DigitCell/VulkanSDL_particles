#ifndef GRAPHMODULE_HPP
#define GRAPHMODULE_HPP

#include "nvulkanbase.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <vector>
#include <exception>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VulkanglTFModel.h"

#include <vulkan/vulkan.h>
#include "ndearimgui.hpp"

#include "supportStructs.hpp"
#include "solver.hpp"

#include "Constants.hpp"




#define ENABLE_VALIDATION false

//#define INSTANCE_COUNT 256
// Set to "true" to enable Vulkan's validation layers (see vulkandebug.cpp for details)
#define ENABLE_VALIDATION false
// Set to "true" to use staging buffers for uploading vertex and index data to device local memory
// See "prepareVertices" for details on what's staging and on why to use it
#define USE_STAGING true



inline void MainGUI(nDearImgui& dearImgui, SimCommandParametrs& simCommandParams, SimDynamicParametrs& sim_parameters) {

     ImGui::Begin("Sim commands");
         ImGui::Text("Framerate  : %.1f ms or %.1f Hz", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
         ImGui::Separator();

         ImGui::Checkbox("Run loop", &simCommandParams.runLoop);

         ImGui::Separator();
         ImGui::Text("Num of Particles %i", sim_parameters.numParticles);
         ImGui::Text("Num of Constraints %i", sim_parameters.constraintsNum);
         ImGui::Text("Num of PolygonsSource %i", sim_parameters.polygonsNum);
         ImGui::Text("Tick %i", sim_parameters.tick);

         ImGui::Checkbox("Draw Particles", &simCommandParams.drawParticles);
         ImGui::Checkbox("Draw Constraints", &simCommandParams.drawConstraints);

         ImGui::Checkbox("Draw Polygons", &simCommandParams.drawPolygons);
         ImGui::Checkbox("Draw Triangles", &simCommandParams.drawTriangles);

          ImGui::SliderInt("Number Colors",&simCommandParams.numColors, 1, 256);

         bool checkRemove=false;
         if(sim_parameters.removeOneEndConstraints==1)
             checkRemove=true;
         ImGui::Checkbox("Remove OneEndConstraints", &checkRemove);
         if(checkRemove==false)
             sim_parameters.removeOneEndConstraints=0;
         else
             sim_parameters.removeOneEndConstraints=1;

         ImGui::SliderFloat("Line Width",&simCommandParams.lineWidth, 1.0f, 7.0f);
         ImGui::SliderFloat("Offset size",&simCommandParams.offsetSize, 1.0f, 17.0f);


         if(ImGui::Button("Reset sim") and simCommandParams.resetSim==false)
         {
                 simCommandParams.resetSim=true;
                 //sim_parameters.particlesNum=0;
                 //sim_parameters.constraintsNum=0;
                 //solver.clearConstraints(sim_parameters);
                 //solver.addParticles(3,sim_parameters);
         }

         if(ImGui::Button("Add partilces") and simCommandParams.addParticles==false)
         {
             simCommandParams.addParticles=true;
             //sim_parameters.particlesNum=0;
             //sim_parameters.constraintsNum=0;
             //solver.clearConstraints(sim_parameters);
             //solver.addParticles(3,sim_parameters);
         }

         if(ImGui::Button("Calculate polygons"))
         {
                 simCommandParams.readPolygons=true;
                 simCommandParams.runLoop=false;
         }

         if(ImGui::Button("Offset polygons"))
         {
                 simCommandParams.offsetPolygons=true;
                 simCommandParams.runLoop=false;
         }

         if(ImGui::Button("Triangulate polygons"))
         {
                 simCommandParams.triangulate=true;
                 simCommandParams.runLoop=false;
         }
         if(ImGui::Button("Save Image"))
         {
             simCommandParams.saveImage=true;
         }

     ImGui::End();


     ImGui::Begin("Sim parameters");

         ImGui::SliderFloat("Max velocity length",&sim_parameters.maxVelocity, 0.0f, 50.0f);
         ImGui::SliderFloat("Velocity dumping",&sim_parameters.velocityDamping, 0.0f, 1.0f);
         ImGui::SliderFloat("Particle radius",&sim_parameters.radius, 0.25f, 15.0f);
        // ImGui::SliderFloat("Voronoi radius",&solver.voronoi_scaleradius, 0.15f, 5.0f);
         ImGui::SliderFloat("Stiffness Coeff",&sim_parameters.stiffnessCoeff, 0.00f, 0.5f);
         ImGui::SliderFloat("Damping Coeff",&sim_parameters.dampingCoeff, 0.0f, 1.5f);

         ImGui::SliderFloat("Constraints dist", &sim_parameters.constraintDist, 1.0f, 100.0f);
         ImGui::SliderFloat("Init constraints dist Start", &sim_parameters.initConstraintDistStart, 1.0f, 100.0f);
         ImGui::SliderFloat("Init constraints dist Stop", &sim_parameters.initConstraintDistStop, 1.0f, 100.0f);
         ImGui::SliderFloat("Break constraints dist",&sim_parameters.breakConstraintDist, 0.0f, 100.0f);

         //solver.radius=sim_parameters.radius;
         ImGui::SliderFloat("Noise Coeff",&sim_parameters.noiseCoeff, 0.0f, 2.5f);
         ImGui::SliderFloat("delta time",&sim_parameters.dt, 0.0f, 1.5f);
         //ImGui::SliderFloat("Scale Window 01",&scaleWindow01, 0.1f, 1.5f);
         //ImGui::SliderFloat("Scale Window 02",&scaleWindow02, 0.1f, 1.5f);
         ImGui::SliderFloat("Color Decrease",&sim_parameters.colorDecrease, 0.001f, 0.01f);
         ImGui::SliderFloat("Scale Window Draw Particles",&simCommandParams.zoomCoeffParticleDraw, 0.5f, 2.0f);
          ImGui::SliderFloat("Scale Window Draw Constraints",&simCommandParams.zoomCoeffConstraintDraw, 0.5f, 2.0f);

     ImGui::End();



     ImGui::Begin("Map View");
         ImGui::Image(dearImgui.imageTemp , ImVec2(SIM_WIDTH/2,SIM_HEIGHT/2));
     ImGui::End();

     ImGui::Begin("Particles View");
         ImGui::Image(dearImgui.imageTemp2 , ImVec2(simCommandParams.zoomCoeffConstraintDraw*SIM_WIDTH/2,simCommandParams.zoomCoeffConstraintDraw*SIM_HEIGHT/2));
     ImGui::End();

     ImGui::Begin("Constraints View");
         ImGui::Image(dearImgui.imageConstraints , ImVec2(simCommandParams.zoomCoeffConstraintDraw*SIM_WIDTH/2,simCommandParams.zoomCoeffConstraintDraw*SIM_HEIGHT/2));
     ImGui::End();

     ImGui::Begin("Particle offscreenRender");
         ImGui::Image(dearImgui.imageParticlesRender , ImVec2(simCommandParams.zoomCoeffParticleDraw*SIM_WIDTH/2,simCommandParams.zoomCoeffParticleDraw*SIM_HEIGHT/2));
     ImGui::End();
/*
     ImGui::Begin("compute draw Particle");
         ImGui::Image(dearImgui.computeDrawParticles , ImVec2(simCommandParams.zoomCoeffConstraintDraw*SIM_WIDTH/2,simCommandParams.zoomCoeffConstraintDraw*SIM_HEIGHT/2));
     ImGui::End();
     */
 };

class GraphModule:nVulkanBase
{
public:

    PlatformSDL& getPlatform() { return nVulkanBase::getPlatform();}

    void prepareFrame() { nVulkanBase::prepareFrame();}
    void submitFrame()  { nVulkanBase::submitFrame();}
    void handleMouseMove() { nVulkanBase::handleMouseMove();}

    void StartTimer() {nVulkanBase::platform.StartTimer();};
    void TimerDelay() {nVulkanBase::platform.TimerDelay();};

    nDearImgui ndearImgui;
    SimCommandParametrs simCommandParams;
    SimParametrs simParams;
    SimDynamicParametrs simDynamicParametrs;
    Solver solver;


    struct {
        uint32_t queueFamilyIndex;
        struct {
            vks::Texture2D rocks;
        } textures;

       // struct {
       //     vkglTF::Model rock;
       // } models;

        struct Quard {
            vks::Buffer vertexBuffer;
            vks::Buffer indexBuffer;
            uint32_t    indexCount;
        } quard;

        struct Circle {
            vks::Buffer vertexBuffer;
            vks::Buffer indexBuffer;
            uint32_t    indexCount;
        } circle;

        vks::Buffer instanceBuffer;
        vks::Buffer instanceConstraints;
        vks::Buffer instancePolygons;
        vks::Buffer drawConstraints;
        vks::Buffer drawPolyline;
        vks::Buffer drawTriangle;


        struct UBOVS {
            glm::mat4 projection;
            glm::mat4 view;
            glm::vec4 lightPos = glm::vec4(0.0f, 3500.0f, 0.0f, 1.0f);

            glm::vec2 worldSize;
            glm::vec2 mapSize;

            float locSpeed = 0.0f;
            float globSpeed = 0.0f;
        } uboVS;

        struct ConstraintUBO {
            glm::vec2 worldSize;
            glm::vec2 mapSize;
        } constraintUBO;

        struct {
            vks::Buffer scene;
            vks::Buffer constraint_scene;
        } uniformBuffers2;

        vks::Buffer uniformBufferVS;

        struct {
            glm::mat4 projection;
            glm::mat4 modelView;
        } uboVS2;

        VkPipelineLayout pipelineLayout;

        VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
        struct {
            VkPipeline instancedParticles;
            VkPipeline instancedConstraints;
            VkPipeline drawConstraints;
            VkPipeline drawPolyline;
            VkPipeline drawTriangles;
        } pipelines;

        VkDescriptorSetLayout descriptorSetLayout;
        struct {
            VkDescriptorSet instancedParticles;
            VkDescriptorSet instancedConstraints;
            VkDescriptorSet drawConstraints;
        } descriptorSets;

    } nGraphics;


    struct {
        uint32_t queueFamilyIndex;
        struct Quard {
            vks::Buffer vertexBuffer;
            vks::Buffer indexBuffer;
            uint32_t indexCount;
        } quard;

        struct {
            vks::Buffer scene;
        } uniformBuffers2;

        VkPipelineLayout pipelineLayout;
        VkSemaphore semaphore;
        VkPipeline pipeline;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
    } mapGraphics;

    struct  {

        uint32_t queueFamilyIndex;
        VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
        VkCommandPool commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)

        struct{
            vks::Buffer simParamsData;
            vks::Buffer simDynamicParamsData;

        } uniforms;

        struct {
            vks::Buffer particlesBuffer;					// (Shader) storage buffer object containing the particles
            vks::Buffer mapData;
            vks::Buffer mapSize;

            vks::Buffer neightb;
            vks::Buffer neightbSort;
            vks::Buffer neightbInfo;

            vks::Buffer constraintsData;
            vks::Buffer constraintsSize;
            vks::Buffer constraintNum;

            vks::Buffer creatureData;

        } buffers;

        struct Compute {

            VkCommandBuffer commandBuffer;				// Command buffer storing the dispatch commands and barriers
            VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
            VkDescriptorSetLayout descriptorSetLayout;	// Compute shader binding layout
            VkDescriptorSet descriptorSet;				// Compute shader bindings
            VkPipelineLayout pipelineLayout;			// Layout of the compute pipeline
            VkPipeline pipeline;			// Compute pipelines for image filters
        } drawParticles;

        Compute clearImage;
        Compute updateParticles;

        Compute clearMap;
        Compute updateMap;

        Compute neightbUpdate;
        Compute neightbSort;

        Compute constraintsClear;
        Compute constraintsUpdate;

    } computeBlock;


    // Resources for the graphics part of the example
    struct {
        struct {
            vks::Texture2D textureColorMap;
            vks::Texture2D textureComputeTarget;
            vks::Texture2D textureComputeTarget2;
            vks::Texture2D textureConstraints;
            vks::Texture2D textureConstraintsCPUFill;
            vks::Texture2D computeDrawParticles;
        } textures;

        //uint32_t queueFamilyIndex;
        VkDescriptorSetLayout descriptorSetLayout;	// Image display shader binding layout
        VkDescriptorSet descriptorSetPreCompute;	// Image display shader bindings before compute shader image manipulation
        VkDescriptorSet descriptorSetPostCompute;	// Image display shader bindings after compute shader image manipulation
        VkPipeline pipeline;						// Image display pipeline
        VkPipelineLayout pipelineLayout;			// Layout of the graphics pipeline
        VkSemaphore semaphore;                      // Execution dependency between compute & graphic submission
    } graphics;


    // Framebuffer for offscreen rendering
    struct FrameBufferAttachment {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    };

    struct OffscreenPass {
        int32_t width, height;
        VkFramebuffer frameBuffer;
        FrameBufferAttachment color, depth;
        VkRenderPass renderPass;
        VkSampler sampler;
        VkDescriptorImageInfo descriptor;
    } offscreenPass;

    OffscreenPass offscreenPassConstraints;
    OffscreenPass offscreenPassParticles;

    GraphModule() : nVulkanBase()
    {
        title = "Instanced mesh rendering";
        camera.type = Camera::CameraType::lookat;
        camera.setPosition(glm::vec3(0.500f, 0.0f, -2515.0f));
        camera.setRotation(glm::vec3(-90 ,0 ,0 ));
        camera.setPerspective(30.0f, (float)width / (float)height, 1.0f, 5556.0f);
    }

    ~GraphModule()
    {
        vkDestroyPipeline(device, nGraphics.pipelines.instancedParticles, nullptr);
        vkDestroyPipelineLayout(device, nGraphics.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, nGraphics.descriptorSetLayout, nullptr);

        // Compute Buffers
        computeBlock.buffers.particlesBuffer.destroy();

        // Compute UpdateParticles
        vkDestroyPipelineLayout(device, computeBlock.updateParticles.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, computeBlock.updateParticles.descriptorSetLayout, nullptr);
        vkDestroyPipeline(device, computeBlock.updateParticles.pipeline, nullptr);
        vkDestroySemaphore(device, computeBlock.updateParticles.semaphore, nullptr);
        vkDestroyCommandPool(device, computeBlock.commandPool, nullptr);

        // Compute drawParticles
        vkDestroyPipelineLayout(device, computeBlock.drawParticles.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, computeBlock.drawParticles.descriptorSetLayout, nullptr);
        vkDestroyPipeline(device, computeBlock.drawParticles.pipeline, nullptr);
        vkDestroySemaphore(device, computeBlock.drawParticles.semaphore, nullptr);
        //vkDestroyCommandPool(device, computeBlock.drawParticles.commandPool, nullptr);

        // Compute clearImage
        vkDestroyPipelineLayout(device, computeBlock.clearImage.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, computeBlock.clearImage.descriptorSetLayout, nullptr);
        vkDestroyPipeline(device, computeBlock.clearImage.pipeline, nullptr);
        vkDestroySemaphore(device, computeBlock.clearImage.semaphore, nullptr);
        //vkDestroyCommandPool(device, computeBlock.clearImage.commandPool, nullptr);

        // Compute clearMap
        vkDestroyPipelineLayout(device, computeBlock.clearMap.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, computeBlock.clearMap.descriptorSetLayout, nullptr);
        vkDestroyPipeline(device, computeBlock.clearMap.pipeline, nullptr);
        vkDestroySemaphore(device, computeBlock.clearMap.semaphore, nullptr);
        //vkDestroyCommandPool(device, computeBlock.clearMap.commandPool, nullptr);

        // Compute updateMap
        vkDestroyPipelineLayout(device, computeBlock.updateMap.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, computeBlock.updateMap.descriptorSetLayout, nullptr);
        vkDestroyPipeline(device, computeBlock.updateMap.pipeline, nullptr);
        vkDestroySemaphore(device, computeBlock.updateMap.semaphore, nullptr);
       // vkDestroyCommandPool(device, computeBlock.updateMap.commandPool, nullptr);


        vkDestroyBuffer(device, nGraphics.instanceBuffer.buffer, nullptr);
        vkFreeMemory(device, nGraphics.instanceBuffer.memory, nullptr);
        //nGraphics.textures.rocks.destroy();
       // nGraphics.uniformBuffers2.scene.destroy();
    }

    void buildCommandBuffers();

    void buildCommandBuffer(int ibuffer);
    void loadAssets();
    void setupDescriptorPool();
    void setupDescriptorSetLayout();
    void setupDescriptorSet();

    void prepareParticlePipeline();
    void prepareConstraintsPipeline();

    void prepareInstanceData();
    void prepareUniformBuffers();
    void updateUniformBuffer(bool viewChanged);
    void draw();
    void prepare();
    void prepareSimParameters();

    void generateQuad();
    void generateCircle();

    void prepareTextureTarget(vks::Texture *tex, uint32_t width, uint32_t height, VkFormat format, uint32_t widthIN, uint32_t heighIN);
    std::vector<std::string> shaderNames;

    void prepareGraphics();
    void prepareparticlesBuffers2();


    void prepareCompute3();
    void buildComputeCommandBuffer3();
    void prepareOffscreen();

    void prepareClearMap();
    void buildClearMapCommandBuffer();

    void prepareUpdateMap();
    void buildUpdateMapCommandBuffer();
    void generateQuadSimple();
    void prepareMapPipeline();

    void prepareComputeDrawParticles();
    void buildComputeDrawParticlesCommandBuffer();

    void prepareComputeUpdateParticles();
    void buildComputeUpdateParticlesCommandBuffer();

    void prepareComputeClearImage();
    void buildComputeClearImageCommandBuffer();

    void prepareNeightbUpdate();
    void buildNeightbUpdateCommandBuffer();

    void prepareNeightbSort();
    void buildNeightbSortCommandBuffer();

    void prepareConstraintsClear();
    void buildConstraintsClearCommandBuffer();

    void prepareConstraintsUpdate();
    void buildConstraintsUpdateCommandBuffer();



    void writeUniformBuffer();

    void prepareComputeBuffers();
    void initParticlesBuffer();
    void addParticlesToBuffer(int num_Particles);
    void resetSim(int num_Particles);
    void initCreaturesBuffer();

    void prepareOffscreenRenderConstraints();
    void prepareDrawConstraintsPipeline();
    void prepareOffscreenRenderParticles();
    void ReadPolygons();
    void prepareDrawPolylinePipeline();
    void OffsetPolygons();
    void TriangulatePolygons();
    void prepareDrawTrianglePipeline();

    bool screenshotSaved = false;
    void saveScreenshot(const char *filename);
};
#endif // GRAPHMODULE_HPP
