#include "nvulkanbase.hpp"

nVulkanBase::nVulkanBase()
{
    initVulkan();
    prepare();
}

void nVulkanBase::getEnabledFeatures() {}

VkResult nVulkanBase::createInstance(bool enableValidation)
{
    this->settings.validation = enableValidation;

    // Validation can also be forced via a define
#if defined(_VALIDATION)
    this->settings.validation = true;
#endif

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.pEngineName = name.c_str();
    appInfo.apiVersion = apiVersion;

    std::vector<const char*> instanceExtensions ={ VK_KHR_SURFACE_EXTENSION_NAME  };// { VK_KHR_SURFACE_EXTENSION_NAME };

    // Enable surface extensions depending on os

//#elif defined(VK_USE_PLATFORM_XCB_KHR)
    instanceExtensions.push_back("VK_KHR_xlib_surface");

    instanceExtensions.push_back("VK_EXT_debug_report");
    instanceExtensions.push_back("VK_EXT_debug_utils");

    // Get extensions supported by the instance and store for later use
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0)
    {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
        {
            for (VkExtensionProperties extension : extensions)
            {
                supportedInstanceExtensions.push_back(extension.extensionName);
            }
        }
    }

    // Enabled requested instance extensions
    if (enabledInstanceExtensions.size() > 0)
    {
        for (const char * enabledExtension : enabledInstanceExtensions)
        {
            // Output message if requested extension is not available
            if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end())
            {
                std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
            }
            instanceExtensions.push_back(enabledExtension);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (instanceExtensions.size() > 0)
    {
        if (settings.validation)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    }

    // The VK_LAYER_KHRONOS_validation contains all current validation functionality.
    // Note that on Android this layer requires at least NDK r20
    const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
    if (settings.validation)
    {
        // Check if this layer is available at instance level
        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
        bool validationLayerPresent = false;
        for (VkLayerProperties layer : instanceLayerProperties) {
            if (strcmp(layer.layerName, validationLayerName) == 0) {
                validationLayerPresent = true;
                break;
            }
        }
        if (validationLayerPresent) {
            instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
            instanceCreateInfo.enabledLayerCount = 1;
        } else {
            std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
        }
    }
    return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}

bool nVulkanBase::initVulkan()
{
    VkResult err;

    // Vulkan instance
    err = createInstance(settings.validation);
    if (err) {
        vks::tools::exitFatal("Could not create Vulkan instance : \n" + vks::tools::errorString(err), err);
        return false;
    }

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    vks::android::loadVulkanFunctions(instance);
#endif

    // If requested, we enable the default validation layers for debugging
    if (settings.validation)
    {
        // The report flags determine what type of messages for the layers will be displayed
        // For validating (debugging) an application the error and warning bits should suffice
        VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        // Additional flags include performance info, loader and layer debug messages, etc.
        vks::debug::setupDebugging(instance, debugReportFlags, VK_NULL_HANDLE);
    }

    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
    if (gpuCount == 0) {
        vks::tools::exitFatal("No device with Vulkan support found", -1);
        return false;
    }
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
    if (err) {
        vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(err), err);
        return false;
    }

    // GPU selection

    // Select physical device to be used for the Vulkan example
    // Defaults to the first device unless specified by command line
    uint32_t selectedDevice = 0;

    physicalDevice = physicalDevices[selectedDevice];

    // Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    // Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
    getEnabledFeatures();

    // Vulkan device creation
    // This is handled by a separate class that gets a logical device representation
    // and encapsulates functions related to a device
    vulkanDevice = new vks::VulkanDevice(physicalDevice);
    VkResult res = vulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
    if (res != VK_SUCCESS) {
        vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(res), res);
        return false;
    }
    device = vulkanDevice->logicalDevice;

    // Get a graphics queue from the device
    vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);

    // Find a suitable depth format
    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
    assert(validDepthFormat);

    swapChain.connect(instance, physicalDevice, device);

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queue
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been submitted and executed
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;

    return true;
}


void nVulkanBase::prepare()
{
    if (vulkanDevice->enableDebugMarkers) {
        vks::debugmarker::setup(device);
    }
    initSwapchain();
    createCommandPool();
    setupSwapChain();
    createCommandBuffers();
    createSynchronizationPrimitives();
    setupDepthStencil();

    setupRenderPass();
    createPipelineCache();

    setupFrameBuffer();
    settings.overlay = settings.overlay && (!benchmark.active);

    /*
    if (settings.overlay) {
        UIOverlay.device = vulkanDevice;
        UIOverlay.queue = queue;
        UIOverlay.shaders = {
            loadShader(getShadersPath() + "base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
            loadShader(getShadersPath() + "base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
        };
        UIOverlay.prepareResources();
        UIOverlay.preparePipeline(pipelineCache, renderPass);
    }
    */
}

void nVulkanBase::initSwapchain(){
    //#elif defined(VK_USE_PLATFORM_XCB_KHR)
    swapChain.initSurface(platform);
}


void nVulkanBase::createCommandPool(){
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
}

void nVulkanBase::setupSwapChain(){
    swapChain.create(&width, &height, settings.vsync);
}

void nVulkanBase::createCommandBuffers()
{
    // Create one command buffer for each swap chain image and reuse for rendering
    drawCmdBuffers.resize(swapChain.imageCount);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vks::initializers::commandBufferAllocateInfo(
            cmdPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            static_cast<uint32_t>(drawCmdBuffers.size()));

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));
}

void nVulkanBase::createSynchronizationPrimitives(){
    // Wait fences to sync command buffer access
    VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    waitFences.resize(drawCmdBuffers.size());
    for (auto& fence : waitFences) {
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
    }
}

void nVulkanBase::setupDepthStencil(){
    VkImageCreateInfo imageCI{};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = depthFormat;
    imageCI.extent = { width, height, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &depthStencil.image));
    VkMemoryRequirements memReqs{};
    vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);

    VkMemoryAllocateInfo memAllloc{};
    memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllloc.allocationSize = memReqs.size;
    memAllloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAllloc, nullptr, &depthStencil.mem));
    VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.mem, 0));

    VkImageViewCreateInfo imageViewCI{};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = depthStencil.image;
    imageViewCI.format = depthFormat;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCI, nullptr, &depthStencil.view));
}


void nVulkanBase::setupRenderPass()
{
    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format = swapChain.colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}


void nVulkanBase::createPipelineCache(){
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

void nVulkanBase::setupFrameBuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    frameBuffers.resize(swapChain.imageCount);
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        attachments[0] = swapChain.buffers[i].view;
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
    }
}

void nVulkanBase::renderFrame(){
    nVulkanBase::prepareFrame();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    nVulkanBase::submitFrame();
}

std::string nVulkanBase::getWindowTitle(){
    std::string device(deviceProperties.deviceName);
    std::string windowTitle;
    windowTitle = title + " - " + device;
    if (!settings.overlay) {
        windowTitle += " - " + std::to_string(frameCounter) + " fps";
    }
    return windowTitle;
}

void nVulkanBase::destroyCommandBuffers(){
    vkFreeCommandBuffers(device, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
}

std::string nVulkanBase::getShadersPath() const{
    return "../data/glsl/" + shaderDir + "/"; //getAssetPath() + "glsl/" + shaderDir + "/";
}

VkPipelineShaderStageCreateInfo nVulkanBase::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
    shaderStage.module = vks::tools::loadShader(fileName.c_str(), device);
#endif
    shaderStage.pName = "main";
    assert(shaderStage.module != VK_NULL_HANDLE);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

void nVulkanBase::renderLoop()
{


}

void nVulkanBase::nextFrame()
{
    auto tStart = std::chrono::high_resolution_clock::now();
    if (viewUpdated)
    {
        viewUpdated = false;
        viewChanged();
    }

    render();
    frameCounter++;
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    frameTimer = (float)tDiff / 1000.0f;
    camera.update(frameTimer);
    if (camera.moving())
    {
        viewUpdated = true;
    }
    // Convert to clamped timer value
    if (!paused)
    {
        timer += timerSpeed * frameTimer;
        if (timer > 1.0)
        {
            timer -= 1.0f;
        }
    }
    float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count());
    if (fpsTimer > 1000.0f)
    {
        lastFPS = static_cast<uint32_t>((float)frameCounter * (1000.0f / fpsTimer));
#if defined(_WIN32)
        if (!settings.overlay)	{
            std::string windowTitle = getWindowTitle();
            SetWindowText(window, windowTitle.c_str());
        }
#endif
        frameCounter = 0;
        lastTimestamp = tEnd;
    }
    // TODO: Cap UI overlay update rates
    //updateOverlay();
}

void nVulkanBase::prepareFrame(){
    // Acquire the next image from the swap chain
    VkResult result = swapChain.acquireNextImage(semaphores.presentComplete, &currentBuffer);
    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
        windowResize();
    }
    else {
        VK_CHECK_RESULT(result);
    }
}

void nVulkanBase::submitFrame(){
    VkResult result = swapChain.queuePresent(queue, currentBuffer, semaphores.renderComplete);
    if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // Swap chain is no longer compatible with the surface and needs to be recreated
            windowResize();
            return;
        } else {
            VK_CHECK_RESULT(result);
        }
    }
    VK_CHECK_RESULT(vkQueueWaitIdle(queue));
}

nVulkanBase::~nVulkanBase()
{
    // Clean up Vulkan resources
    swapChain.cleanup();
    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    destroyCommandBuffers();
    if (renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, renderPass, nullptr);
    }
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
    }

    for (auto& shaderModule : shaderModules)
    {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }
    vkDestroyImageView(device, depthStencil.view, nullptr);
    vkDestroyImage(device, depthStencil.image, nullptr);
    vkFreeMemory(device, depthStencil.mem, nullptr);

    vkDestroyPipelineCache(device, pipelineCache, nullptr);

    vkDestroyCommandPool(device, cmdPool, nullptr);

    vkDestroySemaphore(device, semaphores.presentComplete, nullptr);
    vkDestroySemaphore(device, semaphores.renderComplete, nullptr);
    for (auto& fence : waitFences) {
        vkDestroyFence(device, fence, nullptr);
    }
/*
    if (settings.overlay) {
        UIOverlay.freeResources();
    }
*/
    delete vulkanDevice;

    if (settings.validation)
    {
        vks::debug::freeDebugCallback(instance);
    }

    vkDestroyInstance(instance, nullptr);

    //platform.ClearEvents();
    //destroy windows from destructor
}

void nVulkanBase::setSize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;
    destWidth = width;
    destHeight = height;
    windowResize();
}

void nVulkanBase::viewChanged() {}

void nVulkanBase::buildCommandBuffers() {}

void nVulkanBase::windowResize()
{
    if (!prepared)
    {
        return;
    }
    prepared = false;
    resized = true;

    // Ensure all operations on the device have been finished before destroying resources
    vkDeviceWaitIdle(device);

    // Recreate swap chain
    width = destWidth;
    height = destHeight;
    setupSwapChain();

    // Recreate the frame buffers
    vkDestroyImageView(device, depthStencil.view, nullptr);
    vkDestroyImage(device, depthStencil.image, nullptr);
    vkFreeMemory(device, depthStencil.mem, nullptr);
    setupDepthStencil();
    for (uint32_t i = 0; i < frameBuffers.size(); i++) {
        vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
    }
    setupFrameBuffer();
/*
    if ((width > 0.0f) && (height > 0.0f)) {
        if (settings.overlay) {
            UIOverlay.resize(width, height);
        }
    }
*/
    // Command buffers need to be recreated as they may store
    // references to the recreated frame buffer
    destroyCommandBuffers();
    createCommandBuffers();
    buildCommandBuffers();

    vkDeviceWaitIdle(device);

    if ((width > 0.0f) && (height > 0.0f)) {
        camera.updateAspectRatio((float)width / (float)height);
    }

    // Notify derived class
    //windowResized();
    viewChanged();

    prepared = true;
}

void nVulkanBase::handleMouseMove()
{

    int32_t dx = (int32_t)mousePos.x - platform.mouseX;
    int32_t dy = (int32_t)mousePos.y - platform.mouseY;

    bool handled = false;

    //if (settings.overlay)
    {
        ImGuiIO& io = ImGui::GetIO();
        handled = io.WantCaptureMouse;
    }

    //mouseMoved((float)x, (float)y, handled);
/*
    if (platform.mouseButtons.handled) {
        mousePos = glm::vec2((float)platform.mouseX, (float)platform.mouseY);
        //return;
    }
*/
    if (platform.mouseButtons.left) {
       // camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
        camera.translate(glm::vec3(-dx * 0.07f, -dy * 0.07f, 0.00f));
        viewUpdated = true;
    }
    if (platform.mouseButtons.right) {
        camera.translate(glm::vec3(-0.0f, 0.0f, dy * .075f));
        viewUpdated = true;
    }
    if (platform.mouseButtons.middle) {
        //camera.translate(glm::vec3(-dx * 0.05f, -dy * 0.05f, 0.05f));
        viewUpdated = true;
    }
    mousePos = glm::vec2((float)platform.mouseX, (float)platform.mouseY);


}
