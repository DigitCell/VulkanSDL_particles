#ifndef NDEARIMGUI_HPP
#define NDEARIMGUI_HPP


#include "imgui.h"
#include "imgui_impl_vulkan.h"
//#include "imgui_impl_sdl.h"

#include "vulkan/vulkan.h"

#include "keycodes.hpp"
#include "VulkanTools.h"
#include "VulkanDebug.h"
//#include "VulkanUIOverlay.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"

#include "FileSystem.h"

#include "VulkanInitializers.hpp"
#include "VulkanTexture.h"

#include  "PlatformSDL.h" //"PlatformSDL.h"


#include "supportStructs.hpp"

class nDearImgui
{
public:
    nDearImgui();

    void Init(
            iPlatform &platform,
            VkInstance& instance,
            VkPhysicalDevice& physicalDevice,
            VkDevice& device,
            VkQueue& queue,
            // Command buffer pool
            VkCommandPool& cmdPool,
            // Global render pass for frame buffer writes
            VkRenderPass& renderPass,
            // Descriptor set pool
            VkDescriptorPool& descriptorPool,
            // Pipeline cache object
            VkPipelineCache& pipelineCache,
            // Wraps the swap chain to present images (framebuffers) to the windowing system
            VulkanSwapChain& swapChain
    );

    VkDescriptorPool createDescriptorPool(VkDevice device);
    VkCommandBuffer allocateCommandBuffer(VkCommandPool commandPool, VkDevice device);
    ~nDearImgui();
    void preDestruct(VkDevice& device);
    void preRender(void (*pfnLayOutGui)(nDearImgui &, SimCommandParametrs& simCommandParams, SimDynamicParametrs& simParams), SimCommandParametrs& simCommandParams, SimDynamicParametrs& simParams, iPlatform &platform);
    void IssueBindAndDrawCommands(VkCommandBuffer &commandBuffer, int bufferIndex);
    void uploadFonts(VkDevice& device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);

    VkResult	err;
    //VkDevice&	device;			// (saved for destruction)
    string		iniFileName;	// https://github.com/ocornut/imgui/issues/454
    VkDescriptorSet registerTexture(vks::Texture2D &textuteImage);

    VkDescriptorSet imageTemp;
    VkDescriptorSet imageTemp2;
    VkDescriptorSet imageConstraints;
    VkDescriptorSet imageConstraintsRender;
    VkDescriptorSet imageParticlesRender;
    VkDescriptorSet computeDrawParticles;

    VkDescriptorSet registerImage(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
};

#endif // NDEARIMGUI_HPP
