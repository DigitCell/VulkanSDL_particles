//
// PlatformSDL.cpp
//	General App Chassis
//
// See matched header file for definitive main comment.
//
// The platform may return references to resources, even e.g. pointer to a string.
// In such cases, a reference is assumed to persist (not be weak or scope-dependent)
//	and its resource freed intelligently.  For example, strings may reside in an
//	executable's fixed data.  If otherwise allocated, Apple uses ARC (automatic
//	reference counting), Windows via managed objects and garbage collection, so
//	ensure the platform layer (if applicable) is built to invoke them.
//
// Created 2/7/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "PlatformSDL.h"

#include "AppConstants.h"


PlatformSDL::PlatformSDL()
{
	namePlatform = "SDL";

	initializeSDL();

	createVulkanCompatibleWindow();

	querySupportedVulkanExtensions();

	displayFoundVulkanExtensions();
}

PlatformSDL::~PlatformSDL()
{
	delete supportedVulkanExtensions;

	SDL_DestroyWindow(pWindow);

	SDL_Quit();
}

// INITIALIZATION ----------------------------------------------------------------------------------

// Start SDL.
//
void PlatformSDL::initializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		Fatal("Fail to Initialize SDL: " + string(SDL_GetError()));

	SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, "2");	// (not seeing this behavior as default! despite FULLSCREEN)
}												// ^ The indicator bar is dim and the first swipe makes it visible and the second swipe performs the "home" action (default for fullscreen applications)

// Create a vulkan window; fatal throw on failure.
//
void PlatformSDL::createVulkanCompatibleWindow()
{
	#if TARGET_OS_IPHONE || TARGET_OS_IPHONESIMULATOR	// TODO: Add ANDROID support
		bool isMobilePlatform = true;
	#else
		bool isMobilePlatform = false;
	#endif

	AppSettings& settings = AppConstants.Settings;
	int winWide = 0, winHigh = 0, winX = INT_MIN, winY = INT_MIN;
	if (settings.isInitialized) {
		winWide = settings.startingWindowWidth;
		winHigh = settings.startingWindowHeight;
		winX = settings.startingWindowX;
		winY = settings.startingWindowY;
	}
	if (winWide <= 0 || winWide > AppConstants.MaxSaneScreenWidth)		// Do not allow…
		winWide  = AppConstants.DefaultWindowWidth;
	if (winHigh <= 0 || winHigh > AppConstants.MaxSaneScreenHeight)		//	…window to…
		winHigh = AppConstants.DefaultWindowHeight;
	if (winX < -winWide || winX > AppConstants.MaxSaneScreenWidth)		//	…be entirely…
		winX = SDL_WINDOWPOS_CENTERED;
	if (winY < -winHigh || winY > AppConstants.MaxSaneScreenHeight)		//	…off-screen.
		winY = SDL_WINDOWPOS_CENTERED;

	int windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | (isMobilePlatform ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

	pWindow = SDL_CreateWindow(AppConstants.WindowTitle, winX, winY, winWide, winHigh, windowFlags);
	if (!pWindow)
		Fatal("Fail to Create Vulkan-compatible Window with SDL: " + string(SDL_GetError()));

	pixelsWide = winWide;
	pixelsHigh = winHigh;
	windowX = winX;
	windowY = winY;

	//recordWindowGeometry();		// re-saves anything that had to be "corrected" above
	//No, on 2nd thought, won't.  If re-run, will re-assign the same way.  If user tweaks, then it will save.
	//(plus, the above seems to wipe out the saved credentials, which need to be pulled from Vault if that's to happen)

    //pRenderer = SDL_CreateRenderer(pWindow, 0, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

	SDL_AddEventWatch(realtimeResizingEventWatcher, this);
}

// Finer-granularity callbacks as Window border is grabbed & dragged, making rendering
//	calls to update window content as it is resized.
//	See
//
int PlatformSDL::realtimeResizingEventWatcher(void* data, SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)
	{
		PlatformSDL* pSelf = (PlatformSDL*) data;
		SDL_Window* pEventWindow = SDL_GetWindowFromID(event->window.windowID);
		if (pEventWindow == pSelf->pWindow)
		{
			SDL_GetWindowSize(pEventWindow, &pSelf->pixelsWide, &pSelf->pixelsHigh);

			pSelf->isWindowResized = true;
			// Beware if not properly synchronized with rendering, may crash in vkQueueSubmit() with, e.g.:
			// -[MTLDebugRenderCommandEncoder setScissorRect:]:2702: failed assertion `(rect.x(0) + rect.width(627))(627) must be <= render pass width(623)'

			if (pSelf->pfnResizeForceRender)
				pSelf->pfnResizeForceRender(pSelf->pRenderingObject);

			Log(LOW, "Resize %d x %d... force render.", pSelf->pixelsWide, pSelf->pixelsHigh);
		}
	}
	return 0;
}


void PlatformSDL::createMultiMonitorWindows()
{
	nScreens = SDL_GetNumVideoDisplays();
	screenInfos = new ScreenInfo[nScreens];
	for (int iScreen = 0; iScreen < nScreens; ++iScreen) {
		ScreenInfo& screen = screenInfos[iScreen];
		SDL_GetDisplayBounds(iScreen, &screen.bounds);
		char title[] = "Display 0";
		title[sizeof(title) - 2] = '1' + iScreen;
		screen.pWindow = SDL_CreateWindow(title, screen.bounds.x, screen.bounds.y,
										  screen.bounds.w, screen.bounds.h, SDL_WINDOW_BORDERLESS);
        screen.pRenderer = SDL_CreateRenderer(screen.pWindow, 0, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
		SDL_ShowWindow(screen.pWindow);
	}
}
void PlatformSDL::createVulkanSurface(int iScreen, VkInstance instance, VkSurfaceKHR& surface)
{
	if (SDL_Vulkan_CreateSurface(screenInfos[iScreen].pWindow, instance, &surface))
		return;
	Fatal("Unable to Create Vulkan-compatible Surface using SDL: " + string(SDL_GetError()));
}
void PlatformSDL::destroyMultiMonitorWindows()
{
	for (int iScreen = 0; iScreen < nScreens; ++iScreen) {
		ScreenInfo& screen = screenInfos[iScreen];
		SDL_DestroyRenderer(screen.pRenderer);
		SDL_DestroyWindow(screen.pWindow);
	}
	delete screenInfos;
}
//
// End WIP

// Ask SDL/windowing-system/OS what Vulkan extensions it supports; non-fatally debug-prints success or failure.
//	Expect the default VK_KHR_surface to be returned, accompanied by any platform-specific extensions.
//
void PlatformSDL::querySupportedVulkanExtensions()
{
	ArrayCount nExtensionsReturnedBySDL = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(pWindow, &nExtensionsReturnedBySDL, nullptr))
		Log(ERROR, "Fail to query NUMBER of Vulkan Instance Extensions: " + string(SDL_GetError()));

	nAdditionalExtensions = extendedPlatform.nRequestedExtensionsAvailable();
	nVulkanExtensions = nExtensionsReturnedBySDL + nAdditionalExtensions;

	supportedVulkanExtensions = new StrPtr[nVulkanExtensions];	// Actually get the extensions themselves:
	if (!SDL_Vulkan_GetInstanceExtensions(pWindow, &nExtensionsReturnedBySDL, supportedVulkanExtensions))
		Log(ERROR, "Fail querying " + to_string(nVulkanExtensions) + " Vulkan Instance Extension names: " + string(SDL_GetError()));

	for (Index iAdd = 0, iConcat = nExtensionsReturnedBySDL; iAdd < nAdditionalExtensions && iConcat < nVulkanExtensions; ++iAdd, ++iConcat)
		supportedVulkanExtensions[iConcat] = extendedPlatform.requestedExtensionNames()[iAdd];
}

// Create a Vulkan-compatible surface using the platform layer, throwing fatally on failure.
// Multi-monitor is not supported, as there's but a single SDL_Window pointer.
//
void PlatformSDL::CreateVulkanSurface(VkInstance instance, VkSurfaceKHR& surface)
{
	if (SDL_Vulkan_CreateSurface(pWindow, instance, &surface))
		return;
	Fatal("Unable to Create Vulkan-compatible Surface using SDL: " + string(SDL_GetError()));
}

// Return parameter-pointers set to pixel dimensions of the window or display, or zeros on
//	error (pointed-to ints' values will be destroyed).
//	SDL_GetWindowSize is not a favored method; Apple platforms with SDL_WINDOW_ALLOW_HIGHDPI
//	return rescaled "screen coordinates" not pixels, although it's used as a last resort.
//	For details: https://wiki.libsdl.org/SDL_GetWindowSize
//
bool PlatformSDL::GetWindowSize(int& pixelWidth, int& pixelHeight)
{
	string error;

	pixelWidth = pixelHeight = 0;

	auto pRenderer = SDL_GetRenderer(pWindow);
	if (pRenderer) {
		int result = SDL_GetRendererOutputSize(pRenderer, &pixelWidth, &pixelHeight);
		if (result == 0)  // success
			return true;
		error = "Renderer Output Size returned error";
	} else
		error = "Renderer undefined";
	string sdlError = SDL_GetError();
	if (sdlError != "")
		error += ": " + sdlError;

	// see if possible to return valid values from screen coordinates
	SDL_GetWindowSize(pWindow, &pixelWidth, &pixelHeight);	// (doesn't return a result)
	if (pixelWidth > 0 && pixelHeight > 0) {
		float scaling = getDisplayScaling();
		if (scaling > 0.0f) {
			pixelWidth *= (int) scaling;
			pixelHeight *= (int) scaling;
		}
		return true;
	}
	error += ", Get Window Size returned " + to_string(pixelWidth) + " x " + to_string(pixelHeight);
	sdlError = SDL_GetError();
	if (sdlError != "")
		error += ", " + sdlError;
	Log(WARN, error);
	return false;
}

void PlatformSDL::recordWindowGeometry() // (with logging too)
{
	Log(HANG, "Note: Save Window Geometry: ");

	AppSettings& settings = AppConstants.Settings;
	settings.startingWindowWidth  = pixelsWide;
	settings.startingWindowHeight = pixelsHigh;
	settings.startingWindowX = windowX;
	settings.startingWindowY = windowY;
	settings.Save();
/*
	static int tempPixelsWide = 0,	 // just to make sure not to rewrite file with same values
			   tempPixelsHigh = 0;
	GetWindowSize(tempPixelsWide, tempPixelsHigh);
	if (tempPixelsWide != pixelsWide || tempPixelsHigh != pixelsHigh) {
//		Log(NOTE, "Window Resolution is:  %d x %d", tempPixelsWide, tempPixelsHigh);
		pixelsWide = tempPixelsWide;
		pixelsHigh = tempPixelsHigh;
	}
*/
	//SDL_GEtWindowPosition
}
void PlatformSDL::recordWindowSize(int wide, int high)
{
	if (pixelsWide != wide || pixelsHigh != high) {
		pixelsWide = wide;
		pixelsHigh = high;
		recordWindowGeometry();
	}
}
void PlatformSDL::recordWindowPosition(int x, int y)
{
	if (windowX != x || windowY != y) {
		windowX = x;
		windowY = y;
		recordWindowGeometry();
	}
}

float PlatformSDL::getDisplayScaling()
{
	auto noHiDPI = SDL_GetHintBoolean(SDL_HINT_VIDEO_HIGHDPI_DISABLED, SDL_TRUE);
	if (noHiDPI == SDL_TRUE)
		return 0.0f;
	return getDisplayDPI() / PlatformConstants.DefaultDotsPerInch;
}
//TJ_ADVISORY_NOTE_(DELETE_THIS_AFTER_FURTHER_INVESTIGATION)
// This "Scaling" hasn't been observed to actually work.  Typically pRenderer is NULL, then
//	GetWindowSize returns what look like valid pixel dimensions, yet the scaling value returned
//	is e.g. ~ 1.8, so the dimensions grow big, seeming inaccurate.  The only saving grace is
//	Swapchain.determinSwapExtent 'clamp'ing the values back down to within its 'max' range,
//	which appears to make them "usable" again, whence processing continues normally.
//	It's probably just that I still don't fully understand Apple's SDL_WINDOW_ALLOW_HIGHDPI
//	(in SDL_WindowFlags) but I will... I will have to, eventually.

// Thanks to: https://nlguillemot.wordpress.com/2016/12/11/high-dpi-rendering/
//	Avoid diagonal DPI and don't bother overcomplicating if vertical doesn't match horizontal.
//	(also for Apple .plist note: https://wiki.libsdl.org/SDL_HINT_VIDEO_HIGHDPI_DISABLED )
//
float PlatformSDL::getDisplayDPI(int iDisplay)
{
	float horizontalDPI = 0.0f;		//diagonal				 //vertical
	if (SDL_GetDisplayDPI(iDisplay, nullptr, &horizontalDPI, nullptr) == 0)
		return horizontalDPI;
	return 0.0f;	// indicates error
}


// Inform user via pop-up window.  Defaults to "ERROR" condition (for title and level, see header).
//
void PlatformSDL::DialogBox(const char* message, const char* title, AlertLevel level)
{
	SDL_MessageBoxFlags levels[] = { SDL_MESSAGEBOX_ERROR, SDL_MESSAGEBOX_WARNING, SDL_MESSAGEBOX_INFORMATION };

	SDL_ShowSimpleMessageBox(levels[level], title, message, pWindow);
}

// RUNTIME -----------------------------------------------------------------------------------------

// Polling is the way to go; using SDL_PollEvent gives best responsiveness/performance with
//	one unified IO/Render thread, but even with a multi-threaded/synchronized arrangement.
//	(Note that SDL_Wait blocks indefinitely without returning (seizes rendering) while
//	 SDL_WaitTimeout blocks at a minimum interval of one millisecond, which: is still too
//	 coarse (possibly causing a task-switch), leads to stuttering in rendering or immediate
//	 user-input to visual-output response.  Thus neither Wait method is suitable.)
//
bool PlatformSDL::PollEvent()
{
	static int windowMovedToY, windowMovedToX = INT_MIN;



	if (SDL_PollEvent(&event))
	{
        GUISystemProcessEvent(&event);

		//printf("event %d : %d\n", event.type, event.window.event);

		switch (event.type) {
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_SIZE_CHANGED:		// (and ignoring SDL_WINDOWEVENT_RESIZED, see DEV NOTE 1 at bottom)
						recordWindowSize(event.window.data1, event.window.data2);			// want this to save resized window size to Settings file
						isWindowResized = true;			// (note this remains set until retrieved, whence one-shot resets it)
                        printf("RESIZE %d x %d\n", event.window.data1, event.window.data2);
						Log(LOW, "      Resized %d x %d", pixelsWide, pixelsHigh);	// show resize is finished
						break;
					case SDL_WINDOWEVENT_MOVED:
						windowMovedToX = event.window.data1;	// (see DEV NOTE 2)
						windowMovedToY = event.window.data2;
						Log(LOW, "      Move %d, %d...", windowMovedToX, windowMovedToY);
						return true;
					case SDL_WINDOWEVENT_MINIMIZED:
						isWindowMinimized = true;
						break;
					case SDL_WINDOWEVENT_RESTORED:
						isWindowMinimized = false;
						break;
				}
				break;
			case SDL_MOUSEMOTION:
                mouseButtons.handled = true;
				mouseX = event.motion.x;
				mouseY = event.motion.y;
				break;
            case SDL_MOUSEBUTTONDOWN:
                mouseButtons.handled = true;
                if(event.button.button==SDL_BUTTON_LEFT)
                {
                    mouseButtons.left=true;
                }
                if(event.button.button==SDL_BUTTON_RIGHT)
                {
                    mouseButtons.right=true;
                }
                if(event.button.button==SDL_BUTTON_MIDDLE)
                {
                    mouseButtons.middle=true;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                mouseButtons.handled = false;
                if(event.button.button==SDL_BUTTON_LEFT)
                {
                    mouseButtons.left=false;
                }
                if(event.button.button==SDL_BUTTON_RIGHT)
                {
                    mouseButtons.right=false;
                }
                if(event.button.button==SDL_BUTTON_MIDDLE)
                {
                    mouseButtons.middle=false;
                }
                break;

			default:
				break;
		}
		if (windowMovedToX != INT_MIN) { // Current event was not WINDOW _MOVED, but this indicates that previous event was,
			recordWindowPosition(windowMovedToX, windowMovedToY);	//	therefore "end of dragging around window," so save its position.
			windowMovedToX = INT_MIN;
		}
		return true;
	}
	return false;
}

bool PlatformSDL::IsEventQUIT()
{
	return (event.type == SDL_QUIT
		|| (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
		|| (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE));
}

void PlatformSDL::AwaitEvent()
{
	SDL_WaitEvent(nullptr);
}

void PlatformSDL::ClearEvents()		// This seems good to do prior to main loop for SDL, especially
{									//	since it may unnecessarily start with SDL_WINDOWEVENT_RESIZED.
	while (SDL_PollEvent(&event));
}


/* DEVELOPER NOTE:

SDL_WINDOWEVENT_RESIZED vs. SDL_WINDOWEVENT_SIZE_CHANGED
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 https://wiki.libsdl.org/SDL_WindowEventID says:
 SDL_WINDOWEVENT_RESIZED : window has been resized to data1xdata2; this event is always preceded by SDL_WINDOWEVENT_SIZE_CHANGED.
 SDL_WINDOWEVENT_SIZE_CHANGED : window size has changed, either as a result of an API call or through the system or user changing
								the window size; this event is followed by SDL_WINDOWEVENT_RESIZED if the size was changed by an
								external event, i.e. the user or the window manager.
These speak for themselves.																	  Although, does _RESIZED exclusively
But to reiterate:  Since _RESIZED is *always preceded* by _SIZE_CHANGED, it's not necessary.			 deliver Width x Height ?
That is, only SDL_WINDOWEVENT_SIZE_CHANGED is really necessary to process, especially for our needs.
*/
