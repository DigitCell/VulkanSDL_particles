#include "graphmodule.hpp"


// Prepare a texture target that is used to store compute shader calculations
void GraphModule::prepareTextureTarget(vks::Texture *tex, uint32_t width, uint32_t height, VkFormat format, uint32_t widthIN, uint32_t heightIN)
{
    VkFormatProperties formatProperties;

    // Get device properties for the requested texture format
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
    // Check if requested image format supports image storage operations
    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    // Prepare blit target texture
    tex->width = widthIN;
    tex->height = heightIN;

    VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = { widthIN, heightIN, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imageCreateInfo.flags = 0;
    // If compute and graphics queue family indices differ, we create an image that can be shared between them
    // This can result in worse performance than exclusive sharing mode, but save some synchronization to keep the sample simple
    std::vector<uint32_t> queueFamilyIndices;
    if (vulkanDevice->queueFamilyIndices.graphics != vulkanDevice->queueFamilyIndices.compute) {
        queueFamilyIndices = {
            vulkanDevice->queueFamilyIndices.graphics,
            vulkanDevice->queueFamilyIndices.compute
        };
        imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = 2;
        imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &tex->image));

    vkGetImageMemoryRequirements(device, tex->image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &tex->deviceMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, tex->image, tex->deviceMemory, 0));

    VkCommandBuffer layoutCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    tex->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    vks::tools::setImageLayout(
        layoutCmd, tex->image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        tex->imageLayout);

    vulkanDevice->flushCommandBuffer(layoutCmd, queue, true);

    // Create sampler
    VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    sampler.maxLod = tex->mipLevels;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &tex->sampler));

    // Create image view
    VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
    view.image = VK_NULL_HANDLE;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    view.image = tex->image;
    VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &tex->view));

    // Initialize a descriptor for later use
    tex->descriptor.imageLayout = tex->imageLayout;
    tex->descriptor.imageView = tex->view;
    tex->descriptor.sampler = tex->sampler;
    tex->device = vulkanDevice;
}


void GraphModule::generateQuad()
{
    // Setup vertices for a single uv-mapped quad made from two triangles
    std::vector<VertexPosNormUv> vertices =
    {
        { glm::vec3(  1.0f,  1.0f, 0.0f), glm::vec3( 0.0f, 1.0f, 0.0f ) , glm::vec2( 1.0f, 1.0f )},
        { glm::vec3( -1.0f,  1.0f, 0.0f ),glm::vec3( 0.0f, 1.0f, 0.0f ) , glm::vec2( 0.0f, 1.0f )},
        { glm::vec3( -1.0f, -1.0f, 0.0f ),glm::vec3( 0.0f, 1.0f, 0.0f ) , glm::vec2( 0.0f, 0.0f )},
        { glm::vec3(  1.0f, -1.0f, 0.0f ),glm::vec3( 0.0f, 1.0f, 0.0f ) , glm::vec2( 1.0f, 0.0f )}
    };

    // Setup indices
    std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
    nGraphics.quard.indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers
    // For the sake of simplicity we won't stage the vertex data to the gpu memory
    // Vertex buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &nGraphics.quard.vertexBuffer,
        vertices.size() * sizeof(VertexPosNormUv),
        vertices.data()));
    // Index buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &nGraphics.quard.indexBuffer,
        indices.size() * sizeof(uint32_t),
        indices.data()));
}


void GraphModule::generateQuadSimple()
{
    // Setup vertices for a single uv-mapped quad made from two triangles
    std::vector<glm::vec3> vertices =
    {
        { glm::vec3(  1.0f,  1.0f, 0.0f)},
        { glm::vec3( -1.0f,  1.0f, 0.0f )},
        { glm::vec3( -1.0f, -1.0f, 0.0f )},
        { glm::vec3(  1.0f, -1.0f, 0.0f )}
    };

    // Setup indices
    std::vector<uint32_t> indices = { 0,2,1, 2,0,3 };
    mapGraphics.quard.indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers
    // For the sake of simplicity we won't stage the vertex data to the gpu memory
    // Vertex buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &mapGraphics.quard.vertexBuffer,
        vertices.size() * sizeof(glm::vec3),
        vertices.data()));
    // Index buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &mapGraphics.quard.indexBuffer,
        indices.size() * sizeof(uint32_t),
        indices.data()));
}

void GraphModule::generateCircle()
{
    // Setup vertices for a single uv-mapped quad made from two triangles
    int circleResolution=32;
    float scale=1.0f;
    std::vector<uint32_t> indexBuffer;
    std::vector<VertexPosNormUv> vertexBuffer;

    for (uint32_t i=0; i <=circleResolution+1; ++i)
    {
         float angle = 2 * M_PI * i / circleResolution;
         float x= scale*cos(angle);
         float y= scale*sin(angle);
        //float z=0;

         VertexPosNormUv ttempVertex;
         ttempVertex.pos=     glm::vec3(  x,  0.0f, y);
         ttempVertex.normal=  glm::vec3(0.0f, 1.0f, 0.0f);
         //ttempVertex.color=   glm::vec3(1.0f, 1.0f, 1.0f);

         ttempVertex.uv=glm::vec2(0.5f +x/(2.0f*scale),
                                  0.5f +y/(2.0f*scale));

         vertexBuffer.push_back(ttempVertex);

         if(i>1) {
             indexBuffer.push_back(0);

             indexBuffer.push_back(i);
             indexBuffer.push_back(i-1);
         }
    }

    // Setup indices
    //std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
    nGraphics.circle.indexCount = static_cast<uint32_t>(indexBuffer.size());

    // Create buffers
    // For the sake of simplicity we won't stage the vertex data to the gpu memory
    // Vertex buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &nGraphics.circle.vertexBuffer,
        vertexBuffer.size() * sizeof(VertexPosNormUv),
        vertexBuffer.data()));
    // Index buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &nGraphics.circle.indexBuffer,
        indexBuffer.size() * sizeof(uint32_t),
        indexBuffer.data()));
}


void GraphModule::saveScreenshot(const char *filename)
    {
        screenshotSaved = false;
        bool supportsBlit = true;

        // Check blit support for source and destination
        VkFormatProperties formatProps;

        // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
        vkGetPhysicalDeviceFormatProperties(physicalDevice, swapChain.colorFormat, &formatProps);
        if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
            std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
            supportsBlit = false;
        }

        // Check if the device supports blitting to linear images
        vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
        if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
            std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
            supportsBlit = false;
        }

        // Source for the copy is the last rendered swapchain image

       // offscreenPassConstraints.descriptor.sampler, offscreenPassConstraints.descriptor.imageView, offscreenPassConstraints.descriptor.imageLayout)
        VkImage srcImage = offscreenPassConstraints.color.image;// swapChain.images[currentBuffer];

        // Create the linear tiled destination image to copy to and to read the memory from
        VkImageCreateInfo imageCreateCI(vks::initializers::imageCreateInfo());
        imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
        // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
        imageCreateCI.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateCI.extent.width = SIM_WIDTH;
        imageCreateCI.extent.height = SIM_HEIGHT;
        imageCreateCI.extent.depth = 1;
        imageCreateCI.arrayLayers = 1;
        imageCreateCI.mipLevels = 1;
        imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
        imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        // Create the image
        VkImage dstImage;
        VK_CHECK_RESULT(vkCreateImage(device, &imageCreateCI, nullptr, &dstImage));
        // Create memory to back up the image
        VkMemoryRequirements memRequirements;
        VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
        VkDeviceMemory dstImageMemory;
        vkGetImageMemoryRequirements(device, dstImage, &memRequirements);
        memAllocInfo.allocationSize = memRequirements.size;
        // Memory must be host visible to copy from
        memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &dstImageMemory));
        VK_CHECK_RESULT(vkBindImageMemory(device, dstImage, dstImageMemory, 0));

        // Do the actual blit from the swapchain image to our host visible destination image
        VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // Transition destination image to transfer destination layout
        vks::tools::insertImageMemoryBarrier(
            copyCmd,
            dstImage,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        // Transition swapchain image from present to transfer source layout
        vks::tools::insertImageMemoryBarrier(
            copyCmd,
            srcImage,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
        if (supportsBlit)
        {
            // Define the region to blit (we will blit the whole swapchain image)
            VkOffset3D blitSize;
            blitSize.x = SIM_WIDTH;
            blitSize.y = SIM_HEIGHT;
            blitSize.z = 1;
            VkImageBlit imageBlitRegion{};
            imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlitRegion.srcSubresource.layerCount = 1;
            imageBlitRegion.srcOffsets[1] = blitSize;
            imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlitRegion.dstSubresource.layerCount = 1;
            imageBlitRegion.dstOffsets[1] = blitSize;

            // Issue the blit command
            vkCmdBlitImage(
                copyCmd,
                srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageBlitRegion,
                VK_FILTER_NEAREST);
        }
        else
        {
            // Otherwise use image copy (requires us to manually flip components)
            VkImageCopy imageCopyRegion{};
            imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.srcSubresource.layerCount = 1;
            imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.dstSubresource.layerCount = 1;
            imageCopyRegion.extent.width = SIM_WIDTH;
            imageCopyRegion.extent.height = SIM_HEIGHT;
            imageCopyRegion.extent.depth = 1;

            // Issue the copy command
            vkCmdCopyImage(
                copyCmd,
                srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageCopyRegion);
        }

        // Transition destination image to general layout, which is the required layout for mapping the image memory later on
        vks::tools::insertImageMemoryBarrier(
            copyCmd,
            dstImage,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        // Transition back the swap chain image after the blit is done
        vks::tools::insertImageMemoryBarrier(
            copyCmd,
            srcImage,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        vulkanDevice->flushCommandBuffer(copyCmd, queue);

        // Get layout of the image (including row pitch)
        VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
        VkSubresourceLayout subResourceLayout;
        vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

        // Map image memory so we can start copying from it
        const char* data;
        vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
        data += subResourceLayout.offset;

        std::ofstream file(filename, std::ios::out | std::ios::binary);

        // ppm header
        file << "P6\n" << SIM_WIDTH << "\n" <<SIM_HEIGHT << "\n" << 255 << "\n";

        // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
        bool colorSwizzle = false;
        // Check if source is BGR
        // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
        if (!supportsBlit)
        {
            std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
            colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), swapChain.colorFormat) != formatsBGR.end());
        }

        // ppm binary pixel data
        for (uint32_t y = 0; y < SIM_HEIGHT; y++)
        {
            unsigned int *row = (unsigned int*)data;
            for (uint32_t x = 0; x < SIM_WIDTH; x++)
            {
                if (colorSwizzle)
                {
                      file.write((char*)row, 1);
                       file.write((char*)row+1, 1);
                    file.write((char*)row+2, 1);


                }
                else
                {
                    file.write((char*)row, 3);
                }
                row++;
            }
            data += subResourceLayout.rowPitch;
        }
        file.close();

        std::cout << "Screenshot saved to disk" << std::endl;

        // Clean up resources
        vkUnmapMemory(device, dstImageMemory);
        vkFreeMemory(device, dstImageMemory, nullptr);
        vkDestroyImage(device, dstImage, nullptr);

        screenshotSaved = true;
    }
