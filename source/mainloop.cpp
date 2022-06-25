#include "mainloop.hpp"


#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

MainLoop::MainLoop()
{

}

bool MainLoop::Init()
{
    /*
            graphModule.prepareSynchronizationPrimitives();
            graphModule.prepareVertices(USE_STAGING);
            graphModule.prepareUniformBuffers();
            graphModule.setupDescriptorSetLayout();
            graphModule.preparePipelines();
            graphModule.setupDescriptorPool();
            graphModule.setupDescriptorSet();
            graphModule.buildCommandBuffers();
            //graphModule.prepared = true;
            */
            graphModule.prepare();
}

void MainLoop::Run()
{
    PlatformSDL& platform=graphModule.getPlatform();
    platform.ClearEvents();

    for (bool quit = false; !quit; )
    {

        // Maintain designated frequency of 5 Hz (200 ms per frame)
        a = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> work_time = a - b;

        if (work_time.count() <15.0f)
        {
            std::chrono::duration<double, std::milli> delta_ms(15.0f - work_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
        }

        b = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> sleep_time = b - a;

        platform.StartTimer();
        while (platform.PollEvent())
            quit = platform.IsEventQUIT();

        if (platform.IsWindowMinimized())
            platform.AwaitEvent();

        //updateRender();
        graphModule.handleMouseMove();

        graphModule.updateUniformBuffer(true);

        Draw();
        platform.TimerDelay();

    }
}

void MainLoop::Draw()
{
    graphModule.draw();
}
