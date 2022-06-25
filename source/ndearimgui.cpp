#include "ndearimgui.hpp"

extern void MainGUI2(nDearImgui&)
{

        ImGui::Begin("Vulkan parameters");
            ImGui::Text("Framerate  : %.1f ms or %.1f Hz", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Separator();
        ImGui::End();
};

static void check_vk_result(VkResult err)
{
    if (err == 0) return;
    printf("VkResult %d\n", err);
    if (err < 0)
        abort();
}

static VkAllocationCallbacks*   g_Allocator = nullptr;

nDearImgui::nDearImgui()
{

}

void nDearImgui::Init(
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

        )
{
    ImGui_ImplVulkanH_Window guiWindow;
    guiWindow.Width			= platform.pixelsWide;
    guiWindow.Height		= platform.pixelsHigh;
    guiWindow.Swapchain		= swapChain.swapChain;
    guiWindow.Surface		= swapChain.getSurface();
    guiWindow.SurfaceFormat = swapChain.surfaceFormat;
    guiWindow.RenderPass	= renderPass;
    guiWindow.PresentMode	= VK_PRESENT_MODE_MAX_ENUM_KHR;
    guiWindow.ClearEnable	= false;	// <-- This keeps GUI from clearing screen before it draws!
    guiWindow.ClearValue	= VkClearValue();
    guiWindow.FrameIndex	= 0;
    guiWindow.ImageCount	= swapChain.imageCount;
    guiWindow.SemaphoreIndex = 0;
    guiWindow.Frames		 = NULL;
    guiWindow.FrameSemaphores = NULL;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    platform.InitGUISystem();	// (e.g. for SDL, should in turn call ImGui_ImplSDL2_InitForVulkan(window);)

    ImGui_ImplVulkan_InitInfo init_info = {
        .Instance		= instance,
        .PhysicalDevice	= physicalDevice,
        .Device			= device,
        .QueueFamily	= swapChain.queueNodeIndex,
        .Queue			= queue,
        .PipelineCache	= VK_NULL_HANDLE,
        .DescriptorPool	= createDescriptorPool(device),
        .MinImageCount	= 2,	//g_MinImageCount,
        .ImageCount		= guiWindow.ImageCount,
        .MSAASamples	= VK_SAMPLE_COUNT_1_BIT,
        .Allocator		= NULL,
        .CheckVkResultFn = check_vk_result,
    };

    ImGui_ImplVulkan_Init(&init_info, renderPass);

    /// Although we set up Vulkan ourselves, Dear ImGui assigns its internal Vulkan-related operational variables here.
    ///  ...including things like its own shader modules and pipeline, which we must let it have independently
    ///  because it operates with its own render parameters, like 2D projection.

    // Get our own CommandBuffer from the existing Pool, for existing Queue:
    VkCommandPool commandPool	= cmdPool;
    VkQueue		  graphicsQueue	= queue;

    VkCommandBuffer commandBuffer = allocateCommandBuffer(commandPool, device);

    uploadFonts(device, commandPool, commandBuffer, graphicsQueue);

    iniFileName = FileSystem::AppLocalStorageDirectory() + io.IniFilename;
    io.IniFilename = iniFileName.c_str();
}


void nDearImgui::preDestruct(VkDevice& device)
{
    err = vkDeviceWaitIdle(device);		// Cleanup
    check_vk_result(err);
}

nDearImgui::~nDearImgui()					// (CommandBuffer should get destroyed when commandPool does.)
{
    ImGui_ImplVulkan_Shutdown();		// Free/destroy internals (like font/descriptor/pipeline
    ImGui_ImplSDL2_Shutdown();			//	or SDL clipboard/mouseCursors) that ImGui allocated.

    ImGui::DestroyContext();
}
/*
void nDearImgui::Update(float deltaSeconds)
{		// (Don't really need deltaSeconds, as Dear ImGui tracks its own time.)

    preRender(MainGUI, platform);

}
*/

VkDescriptorSet nDearImgui::registerTexture(vks::Texture2D& textuteImage)
{
      auto imageView=textuteImage.view;
      auto imageSampler=textuteImage.sampler;
      auto texIdTmp=ImGui_ImplVulkan_AddTexture(imageSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      return texIdTmp;
}

VkDescriptorSet nDearImgui::registerImage(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout)
{

      auto texIdTmp=ImGui_ImplVulkan_AddTexture(sampler, image_view, image_layout);
      return texIdTmp;
}

// Start the Dear ImGui frame ...and Rendering.
//

void nDearImgui::preRender(void(*pfnLayOutGui)(nDearImgui&, SimCommandParametrs& simCommandParams, SimDynamicParametrs& simParams),
                           SimCommandParametrs& simCommandParams, SimDynamicParametrs& simParams, iPlatform &platform)
{
    ImGui_ImplVulkan_NewFrame();
    platform.GUISystemNewFrame();	// (e.g. for SDL, should in turn call ImGui_ImplSDL2_NewFrame(window);)

    ImGui::NewFrame();
        pfnLayOutGui(*this, simCommandParams, simParams);
    ImGui::Render();

}

// Record Imgui Draw Data and draw funcs into command buffer.
//
void nDearImgui::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    //ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
}


// Upload fonts using any command queue... but watch out, it'll kill one that's already set-up.
//
void nDearImgui::uploadFonts(VkDevice& device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    err = vkResetCommandPool(device, commandPool, 0);
    check_vk_result(err);

    VkCommandBufferBeginInfo beginfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    err = vkBeginCommandBuffer(commandBuffer, &beginfo);
    check_vk_result(err);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    VkSubmitInfo endInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };
    err = vkEndCommandBuffer(commandBuffer);
    check_vk_result(err);

    err = vkQueueSubmit(queue, 1, &endInfo, VK_NULL_HANDLE);
    check_vk_result(err);

    err = vkDeviceWaitIdle(device);
    check_vk_result(err);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}


VkCommandBuffer nDearImgui::allocateCommandBuffer(VkCommandPool commandPool, VkDevice device)
{
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo = {
        .sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext	= nullptr,
        .commandPool		= commandPool,
        .level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VkResult err = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    check_vk_result(err);
    return commandBuffer;
}

VkDescriptorPool nDearImgui::createDescriptorPool(VkDevice device)
{
    const int SIZE = 1000;	// arbitrary default pool size

    VkDescriptorPool descriptorPool;
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER,				 SIZE },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SIZE },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,			 SIZE },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,			 SIZE },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,	 SIZE },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,	 SIZE },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,		 SIZE },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,		 SIZE },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SIZE },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, SIZE },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,		 SIZE }
    };
    VkDescriptorPoolCreateInfo pool_info = {
        .sType	 = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags	 = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = SIZE * IM_ARRAYSIZE(pool_sizes),
        .poolSizeCount	= (uint32_t) IM_ARRAYSIZE(pool_sizes),
        .pPoolSizes		= pool_sizes
    };
    VkResult err = vkCreateDescriptorPool(device, &pool_info, g_Allocator, &descriptorPool);
    check_vk_result(err);
    return descriptorPool;
}


/*
 *     void buildCommandBuffers()
    {

        ndearImgui.preRender(MainGUI, getPlatform());

        for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
        {
            // Set target frame buffer
            renderPassBeginInfo.framebuffer = frameBuffers[i];

            VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

            // Start the first sub pass specified in our default render pass setup by the base class
            // This will clear the color and depth attachment
            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), drawCmdBuffers[i]);// One command buffer

            vkCmdEndRenderPass(drawCmdBuffers[i]);

        }
    }


    inline void MainGUI(nDearImgui& dearImgui) {
             ImGui::Begin("Vulkan parameters");
                 ImGui::Text("Framerate  : %.1f ms or %.1f Hz", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                 ImGui::Separator();

             ImGui::End();
             ImGui::Begin("Image View");
           //      ImGui::Image(dearImgui.texIdList[0] , ImVec2(512,512));
             ImGui::End();
     };

    //declare
     nDearImgui ndearImgui;
    //prepare
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

 */
