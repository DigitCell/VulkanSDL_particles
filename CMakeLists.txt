
cmake_minimum_required(VERSION 3.15)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

project(sdl2_vulkan)
set(PROJECT_NAME Vulkan_exp)

include_directories(external/ktx/include)
include_directories(external/ktx/other_include)
include_directories(external/tinygltf)
include_directories(external/imgui)
include_directories(src/base)

find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)

# SDL2, glew, glm, ImGui
#add_subdirectory("external")
#add_subdirectory("external/vk-bootstrap")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")

find_package(nlohmann_json 3.2.0 REQUIRED)

set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ktx)
set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c)

add_library(baseKTX STATIC ${KTX_SOURCES})
target_link_libraries(baseKTX ${Vulkan_LIBRARY} ${XCB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/base)
set(BASE_SOURCES
    ${BASE_DIR}/benchmark.hpp
    ${BASE_DIR}/camera.hpp
    ${BASE_DIR}/frustum.hpp
    ${BASE_DIR}/keycodes.hpp
    ${BASE_DIR}/threadpool.hpp
    ${BASE_DIR}/VulkanAndroid.cpp
    ${BASE_DIR}/VulkanAndroid.h
    ${BASE_DIR}/VulkanBuffer.cpp
    ${BASE_DIR}/VulkanBuffer.h
    ${BASE_DIR}/VulkanDebug.cpp
    ${BASE_DIR}/VulkanDebug.h
    ${BASE_DIR}/VulkanDevice.cpp
    ${BASE_DIR}/VulkanDevice.h
    ${BASE_DIR}/VulkanFrameBuffer.hpp
    ${BASE_DIR}/VulkanglTFModel.cpp
    ${BASE_DIR}/VulkanglTFModel.h
    ${BASE_DIR}/VulkanHeightmap.hpp
    ${BASE_DIR}/VulkanInitializers.hpp
    ${BASE_DIR}/VulkanRaytracingSample.cpp
    ${BASE_DIR}/VulkanRaytracingSample.h
    ${BASE_DIR}/VulkanSwapChain.cpp
    ${BASE_DIR}/VulkanSwapChain.h
    ${BASE_DIR}/VulkanTexture.cpp
    ${BASE_DIR}/VulkanTexture.h
    ${BASE_DIR}/VulkanTools.cpp
    ${BASE_DIR}/VulkanTools.h
    ${BASE_DIR}/VulkanUIOverlay.cpp
    ${BASE_DIR}/VulkanUIOverlay.h)

add_library(base STATIC ${BASE_SOURCES})
target_link_libraries(base ${Vulkan_LIBRARY} ${XCB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

#GLM LIB
set(GLM_DIR  ${CMAKE_SOURCE_DIR}/external/glm-0.9.9.8/cmake/glm)
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})
add_definitions(${GLM_DEFINITIONS})
if(NOT GLM_FOUND)
        message(Error "GLM not found")
endif(NOT GLM_FOUND)

# IMGUI LIB
set (IMGUILIB ImGuiLibrary)
set(IMGUI_ROOT ${CMAKE_SOURCE_DIR}/external/imgui)#-1.92)
set(IMGUI_SOURCES ${IMGUI_ROOT}/imgui.cpp  ${IMGUI_ROOT}/imgui_draw.cpp ${IMGUI_ROOT}/imgui_widgets.cpp ${IMGUI_ROOT}/imgui_tables.cpp)
set(IMGUI_IMPL_SOURCES ${IMGUI_ROOT}/backends/imgui_impl_sdl.cpp  ${IMGUI_ROOT}/backends/imgui_impl_vulkan.cpp)
add_library(${IMGUILIB} STATIC ${IMGUI_SOURCES} ${IMGUI_IMPL_SOURCES} )
target_include_directories(${IMGUILIB} PUBLIC "${IMGUI_ROOT}")
target_include_directories(${IMGUILIB} PUBLIC "${IMGUI_ROOT}/backends")

include_directories(src)
include_directories(src/Settings)
include_directories(src/VulkanModule)
include_directories(src/VulkanModule/AddOns)
include_directories(src/VulkanModule/AddOns/VertexTypes)
include_directories(src/VulkanModule/AddOns/3DObjects)
include_directories(src/VulkanModule/AddOns/Renderables)

include_directories(src/VulkanModule/Assist)
include_directories(src/VulkanModule/Objects)

include_directories(src/VulkanModule/Platform)
include_directories(src/VulkanModule/Platform/FileSystem)
include_directories(src/VulkanModule/Platform/GUISystem)
#include_directories(src/VulkanModule/Platform/GUISystem/replacements)
include_directories(src/VulkanModule/Platform/ImageHandling)
include_directories(src/VulkanModule/Platform/Logger)
include_directories(src/VulkanModule/Platform/OSAbstraction)

include_directories(src/VulkanModule/Setup)
include_directories(src/VulkanModule/nObjects)


SET(TARGET_H

    #src/Settings/
    src/Settings/AppConstants.h
    src/Settings/PlatformConstants.h
    src/Settings/PlatformSpecifics.h
    src/Settings/LocalFileSystem.h

    src/Settings/AppSettings.h
    src/Settings/jsonConvert_c.h
    src/Settings/json.hpp

    src/VulkanModule/AddOns/BufferBase.h
    src/VulkanModule/AddOns/CommandBufferBase.h
    src/VulkanModule/AddOns/Descriptors.h
    src/VulkanModule/AddOns/Mipmaps.h
    src/VulkanModule/AddOns/PrimitiveBuffer.h
    src/VulkanModule/AddOns/TextureImage.h
    src/VulkanModule/AddOns/UniformBuffer.h
    src/VulkanModule/AddOns/UniformBufferLiterals.h

    #src/VulkanModule/AddOns/VertexTypes/

    src/VulkanModule/AddOns/VertexTypes/Vertex2DColored.h
    src/VulkanModule/AddOns/VertexTypes/Vertex2D.h
    src/VulkanModule/AddOns/VertexTypes/Vertex2DTextured.h
    src/VulkanModule/AddOns/VertexTypes/Vertex2DTextureTinted.h
    src/VulkanModule/AddOns/VertexTypes/VertexNull.h
    src/VulkanModule/AddOns/VertexTypes/VertexType.h

   # src/VulkanModule/AddOns/3DObjects/
    src/VulkanModule/AddOns/3DObjects/VertexBasedObject.h

    #src/VulkanModule/AddOns/Renderables/
    src/VulkanModule/AddOns/Renderables/AddOns.h
    src/VulkanModule/AddOns/Renderables/DynamicRenderable.h
    src/VulkanModule/AddOns/Renderables/FixedRenderable.h
    src/VulkanModule/AddOns/Renderables/iRenderable.h

    #src//VulkanModule/Assist/
    src/VulkanModule/Assist/DebugReport.h
    src/VulkanModule/Assist/DeviceAssessment.h
    src/VulkanModule/Assist/DeviceProfile.h
    src/VulkanModule/Assist/DeviceRanking.h
    src/VulkanModule/Assist/Helpers.h
    src/VulkanModule/Assist/Utility.h
    src/VulkanModule/Assist/ValidationLayers.h
    src/VulkanModule/Assist/VulkanPlatform.h

    #src/VulkanModule/Objects/
    src/VulkanModule/Objects/CommandObjects.h
    src/VulkanModule/Objects/DeviceQueues.h
    src/VulkanModule/Objects/EventObjects.h
    src/VulkanModule/Objects/Framebuffers.h
    src/VulkanModule/Objects/GraphicsDevice.h
    src/VulkanModule/Objects/GraphicsPipeline.h
    src/VulkanModule/Objects/RenderPass.h
    src/VulkanModule/Objects/ShaderModules.h
    src/VulkanModule/Objects/Swapchain.h
    src/VulkanModule/Objects/SyncObjects.h
    src/VulkanModule/Objects/VulkanInstance.h
    src/VulkanModule/Objects/WindowSurface.h

    #src/VulkanModule/Platform/
    src/VulkanModule/Platform/PlatformExtension.h
    src/VulkanModule/Platform/Universal.h

    #src/VulkanModule/Platform/FileSystem/
    src/VulkanModule/Platform/FileSystem/FileSystem.h
    src/VulkanModule/Platform/FileSystem/FileSystemSDL.h

    #src/VulkanModule/Platform/GUISystem/
    src/VulkanModule/Platform/GUISystem/DearImGui.h

    #src/VulkanModule/Platform/GUISystem/replacements/
    #src/VulkanModule/Platform/GUISystem/replacements/imgui_impl_vulkan.h
    #src/VulkanModule/Platform/GUISystem/stubs/
    #src/VulkanModule/Platform/GUISystem/stubs/imgui.h
    #src/VulkanModule/Platform/GUISystem/stubs/imgui_impl_sdl.h

    #src/VulkanModule/Platform/ImageHandling/
    src/VulkanModule/Platform/ImageHandling/iImageSource.h
    src/VulkanModule/Platform/ImageHandling/ImageSDL.h
    src/VulkanModule/Platform/ImageHandling/ImageSTB.h
    src/VulkanModule/Platform/ImageHandling/sdlEquivalents.h

    #src/VulkanModule/Platform/Logger/
    src/VulkanModule/Platform/Logger/Logging.h

    #src/VulkanModule/Platform/OSAbstraction/
    #src/VulkanModule/Platform/OSAbstraction/iPlatform.h
    #src/VulkanModule/Platform/OSAbstraction/PlatformGLFW.h
    src/VulkanModule/Platform/OSAbstraction/PlatformSDL.h
    #src/VulkanModule/Platform/OSAbstraction/PlatformXCB.h

    #src/VulkanModule/Setup/
    src/VulkanModule/Setup/RenderSettings.h
    src/VulkanModule/Setup/Shader.h
    src/VulkanModule/Setup/VulkanConfigure.h
    src/VulkanModule/Setup/VulkanSetup.h
    src/VulkanModule/Setup/VulkanSingleton.h

)

SET(TARGET_SRC

    #src/Settings/
    src/Settings/AppSettings.cpp

    src/VulkanModule/AddOns/Descriptors.cpp
    src/VulkanModule/AddOns/PrimitiveBuffer.cpp
    src/VulkanModule/AddOns/TextureImage.cpp
    src/VulkanModule/AddOns/UniformBuffer.cpp

    # src/VulkanModule/AddOns/3DObjects/
    src/VulkanModule/AddOns/3DObjects/Quad2DColored.cpp
    src/VulkanModule/AddOns/3DObjects/Quad2D.cpp
    src/VulkanModule/AddOns/3DObjects/Quad2DTextured.cpp
    src/VulkanModule/AddOns/3DObjects/Quad2DTextureTinted.cpp
    src/VulkanModule/AddOns/3DObjects/Triangle2DColored.cpp

    #src/VulkanModule/AddOns/VertexTypes/
    src/VulkanModule/AddOns/VertexTypes/VertexTypes.cpp

    #src/VulkanModule/AddOns/Renderables/
    src/VulkanModule/AddOns/Renderables/AddOns.cpp
    src/VulkanModule/AddOns/Renderables/DynamicRenderable.cpp
    src/VulkanModule/AddOns/Renderables/FixedRenderable.cpp

    #src//VulkanModule/Assist/
    src/VulkanModule/Assist/DebugReport.cpp
    src/VulkanModule/Assist/DeviceAssessment.cpp
    src/VulkanModule/Assist/DeviceRanking.cpp
    src/VulkanModule/Assist/Utility.cpp
    src/VulkanModule/Assist/ValidationLayers.cpp
    src/VulkanModule/Assist/vkEnumStrings.cpp

    #src/VulkanModule/Objects/
    src/VulkanModule/Objects/CommandObjects.cpp
    src/VulkanModule/Objects/DeviceQueues.cpp
    src/VulkanModule/Objects/EventObjects.cpp
    src/VulkanModule/Objects/Framebuffers.cpp
    src/VulkanModule/Objects/GraphicsDevice.cpp
    src/VulkanModule/Objects/GraphicsPipeline.cpp
    src/VulkanModule/Objects/RenderPass.cpp
    src/VulkanModule/Objects/ShaderModules.cpp
    src/VulkanModule/Objects/Swapchain.cpp
    src/VulkanModule/Objects/SyncObjects.cpp
    src/VulkanModule/Objects/VulkanInstance.cpp
    src/VulkanModule/Objects/WindowSurface.cpp

    #src/VulkanModule/Platform/
    src/VulkanModule/Platform/PlatformCommon.cpp
    #src/VulkanModule/Platform/FileSystem/
    src/VulkanModule/Platform/FileSystem/FileSystem.cpp
    #src/VulkanModule/Platform/GUISystem/
    src/VulkanModule/Platform/GUISystem/DearImGui.cpp

    #src/VulkanModule/Platform/GUISystem/replacements/
    #src/VulkanModule/Platform/GUISystem/replacements/imgui_impl_vulkan.cpp

    #src/VulkanModule/Platform/GUISystem/stubs/
    #src/VulkanModule/Platform/GUISystem/stubs/imgui_impl_sdl.cpp

    #src/VulkanModule/Platform/Logger/
    src/VulkanModule/Platform/Logger/Logging.cpp

    #src/VulkanModule/Platform/OSAbstraction/
    #src/VulkanModule/Platform/OSAbstraction/PlatformGLFW.cpp
    src/VulkanModule/Platform/OSAbstraction/PlatformSDL.cpp

    #src/VulkanModule/Setup/
    src/VulkanModule/Setup/VulkanSetup.cpp
)

add_library(baseSDL STATIC  ${TARGET_SRC} ${TARGET_H})
target_link_libraries(baseSDL ${Vulkan_LIBRARY} ${SDL2_LIBRARIES} -lSDL2_image nlohmann_json::nlohmann_json ${GLM_LIBRARIES})
target_link_libraries(baseSDL ${IMGUILIB} )
target_link_libraries(baseSDL baseKTX)

SET(nTARGET_H

   # src/VulkanModule/nObjects/nrenderable.hpp
   # src/VulkanModule/nObjects/basevulkan.hpp

    source/supportStructs.hpp
    source/graphmodule.hpp
    source/mainloop.hpp
    source/nvulkanbase.hpp
    source/ndearimgui.hpp
    source/Constants.hpp

    source/solver.hpp
    source/support/tinycolormap.hpp


)

SET(nTARGET_SRC

   # src/VulkanModule/nObjects/nrenderable.cpp
   # src/VulkanModule/nObjects/basevulkan.cpp

    source/graphmodule.cpp
    source/mainloop.cpp
    source/nvulkanbase.cpp
    source/ndearimgui.cpp

    source/graphmodule_computepart.cpp
    source/graphmodule_renderpart.cpp
    source/graphmodule_supportpart.cpp
    source/solver.cpp

    main.cpp
)


#find_package(CGAL 5.4)



add_executable(${PROJECT_NAME} ${nTARGET_SRC} ${nTARGET_H})
target_link_libraries(${PROJECT_NAME} ${IMGUILIB} )
#target_link_libraries(${PROJECT_NAME} vk-bootstrap::vk-bootstrap)

set(homeDir "/home/geotyper")

#target_link_libraries(${PROJECT_NAME} CGAL::CGAL)

target_include_directories(${PROJECT_NAME} PUBLIC "${GLM_DIR}")
target_link_libraries(${PROJECT_NAME} baseKTX)
target_link_libraries(${PROJECT_NAME} base)
target_link_libraries(${PROJECT_NAME} baseSDL)
target_link_libraries(${PROJECT_NAME}  nlohmann_json::nlohmann_json)

target_link_libraries(${PROJECT_NAME}  ${SDL2_LIBRARIES} -lSDL2_image)
target_link_libraries(${PROJECT_NAME}  ${Vulkan_LIBRARIES})
target_link_libraries(${PROJECT_NAME}  ${GLM_LIBRARIES})
