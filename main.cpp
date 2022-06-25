#include <iostream>

#include "source/mainloop.hpp"

#include "AppSettings.h"
#include "AppConstants.h"
#include "src/VulkanModule/Platform/Logger/Logging.h"

using namespace std;

int main(int argc, char* argv[])
{
    cout << "Start Vulkan exp application" << endl;

    AppConstants.setExePath(argv[0]);
    LogStartup();

    MainLoop mainLoop;


    try {
         mainLoop.Init();
         mainLoop.Run();

    } catch (const exception& e) {
        const char* message = e.what();
        //mainLoop.DialogBox(message);
        Log(RAW, "FAIL: %s", message);
        return EXIT_FAILURE;
    }

     AppConstants.Settings.Save();

    return EXIT_SUCCESS;
}
