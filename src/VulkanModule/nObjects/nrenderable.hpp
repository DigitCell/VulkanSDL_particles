#ifndef NRENDERABLE_HPP
#define NRENDERABLE_HPP


#include "basevulkan.hpp"
#include "VulkanSetup.h"
#include "VulkanPlatform.h"

#include "GraphicsDevice.h"
#include "ShaderModules.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "VertexType.h"
#include "Descriptors.h"

#include "VertexBasedObject.h"
#include "UniformBufferLiterals.h"
#include "UniformBuffer.h"
#include "TextureImage.h"

#include "VulkanglTFModel.h"

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define ENABLE_VALIDATION false
#if defined(__ANDROID__)
#define INSTANCE_COUNT 4096
#else
#define INSTANCE_COUNT 8192
#endif

class nRenderable: baseVulkan
{
public:
    nRenderable(VulkanSetup& _vulkan, iPlatform& _platform, VkQueue& _deviceQueue):
       nvulkan(_vulkan),
       nplatform(_platform),
       deviceQueue(_deviceQueue)
    {
    };

    VulkanSetup& nvulkan;
    iPlatform& nplatform;
    VkQueue&		deviceQueue;

    struct {
            vks::Texture2DArray rocks;
            vks::Texture2D planet;
        } textures;

        struct {
            vkglTF::Model rock;
            vkglTF::Model planet;
        } models;

        // Per-instance data block
        struct InstanceData {
            glm::vec3 pos;
            glm::vec3 rot;
            float scale;
            uint32_t texIndex;
        };
        // Contains the instanced data
        struct InstanceBuffer {
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            size_t size = 0;
            VkDescriptorBufferInfo descriptor;
        } instanceBuffer;

        struct UBOVS {
            glm::mat4 projection;
            glm::mat4 view;
            glm::vec4 lightPos = glm::vec4(0.0f, -5.0f, 0.0f, 1.0f);
            float locSpeed = 0.0f;
            float globSpeed = 0.0f;
        } uboVS;

        struct {
            vks::Buffer scene;
        } uniformBuffers;

        VkPipelineLayout pipelineLayout;
        struct {
            VkPipeline instancedRocks;
            VkPipeline planet;
            VkPipeline starfield;
        } pipelines;

        VkDescriptorSetLayout descriptorSetLayout;
        struct {
            VkDescriptorSet instancedRocks;
            VkDescriptorSet planet;
        } descriptorSets;







};

#endif // NRENDERABLE_HPP
