//
// PlatformSDL.h
//	General App Chassis
//
// Platform-specific Concretion for Simple DirectMedia Layer.
//
// Feel free to delete the base class/interface if you'll only ever use SDL.
//	An abstraction with one concretion is unnecessary cruft; simpler is better!
//
// TODO: Add multi-monitor (multi-window) support, eventually.
//
// Created 2/7/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef PlatformSDL_h
#define PlatformSDL_h

// Avoid SDL's many warnings: Empty paragraph passed to '\param' command
#pragma clang diagnostic ignored "-Wdocumentation"
#include <string.h>		// <--(otherwise 'memcpy' may warn "use of undeclared identifier" in SDL_stdinc.h)
#include <SDL.h>
#include <SDL_vulkan.h>

#include "iPlatform.h"
#include "ImageSDL.h"

#include "imgui.h"
#include "../external/imgui/backends/imgui_impl_sdl.h"
#include <bits/stdc++.h>

#include "AppConstants.h"


// Work In Progress ... MULTI-MONITOR SUPPORT
//	These methods seem to work, but:
//	TODO: needs method to specify using: windowed vs. full-screen vs. how many full-screen-displays
//		obviously this needs combined with the isMobilePlatform stuff above
//		note that SDL_WINDOW_BORDERLESS may need to supersede SDL_WINDOW_FULLSCREEN for multimon to work
//
struct ScreenInfo {
    SDL_Window*		pWindow;
    SDL_Rect		bounds;
    SDL_Renderer*	pRenderer;
};


class PlatformSDL : public iPlatform
{
public:
	PlatformSDL();
	~PlatformSDL();

		// MEMBERS
private:
	SDL_Window*  pWindow;	// Note: only supports a single window, hence single monitor.
	SDL_Event	 event;
	ImageSDL	 image;
    SDL_Renderer*	pRenderer;

    int nScreens = 0;
    ScreenInfo* screenInfos;

	typedef void (*PFNResizeForceRender)(void* pObject);
	PFNResizeForceRender pfnResizeForceRender = nullptr;
	void* pRenderingObject = nullptr;

		// METHODS
public:
	void CreateVulkanSurface(VkInstance instance, VkSurfaceKHR& surface);
	bool GetWindowSize(int& pixelWidth, int& pixelHeight);
	void DialogBox(const char* message, const char* title = "ERROR", AlertLevel level = FAILURE);
	bool PollEvent();
	bool IsEventQUIT();
	void AwaitEvent();
	void ClearEvents();

	iImageSource& ImageSource()	 { return static_cast<iImageSource&>(image); }
    void InitGUISystem()
    {
        ImGui_ImplSDL2_InitForVulkan(pWindow);
    }
    void GUISystemNewFrame()
    {
        ImGui_ImplSDL2_NewFrame(pWindow);
    }
	void GUISystemProcessEvent(SDL_Event* pEvent)
								 { ImGui_ImplSDL2_ProcessEvent(pEvent); }
	void RegisterForceRenderCallback(PFNResizeForceRender pfnForceRender, void* pObject) {
									pfnResizeForceRender = pfnForceRender;
									pRenderingObject = pObject;
								 }

    void StartTimer() {startTimer = SDL_GetPerformanceCounter();};
    void TimerDelay() {
            endTimer = SDL_GetPerformanceCounter();
            float elapsedMS = (endTimer - startTimer) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
            // Cap to 60 FPS
            //SDL_Delay(floor(16.666f - elapsedMS));

    }



    struct {
        bool handled = false;
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

private:
	void initializeSDL();
	void createVulkanCompatibleWindow();
	void querySupportedVulkanExtensions();
	void recordWindowGeometry();
	void recordWindowSize(int wide, int high);
	void recordWindowPosition(int x, int y);
	float getDisplayScaling();
	float getDisplayDPI(int iDisplay = 0);

	// WIP, TODO: "more officially" integrate later
	void createMultiMonitorWindows();
	void createVulkanSurface(int iScreen, VkInstance instance, VkSurfaceKHR& surface);
	void destroyMultiMonitorWindows();

	static int realtimeResizingEventWatcher(void* data, SDL_Event* event);

    Uint64 startTimer;
    Uint64 endTimer;



};

#endif	// PlatformSDL_h
