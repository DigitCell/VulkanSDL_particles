#ifndef MAINLOOP_HPP
#define MAINLOOP_HPP

#include "graphmodule.hpp"

class MainLoop
{

private:


public:
    //nVulkanBase nvulkanBase;
    GraphModule graphModule;
    MainLoop();
    bool Init();
    void Run();
    void Draw();
};

#endif // MAINLOOP_HPP
