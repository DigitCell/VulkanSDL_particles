//
// AppConstants.h
//	VulkanSDL Application Chassis
//
// Created 2/17/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef AppConstants_h
#define AppConstants_h

#ifdef INSTANTIATE
  #define extern
#endif

#include "PlatformConstants.h"
#include "VulkanPlatform.h"
#include "AppSettings.h"


extern struct Constants
{
    const StrPtr AppName			= "VulkanSDL_experiments";
    const uint32_t AppVersion		= VK_MAKE_VERSION(1, 3, 204);		// (Vulkan wants this)

    const StrPtr WindowTitle		= "Vulkan SDL Exp";

	const StrPtr SettingsFileName	= "Settings.json";

	const StrPtr DebugLogFileName	= "DebugLog.txt";

    const StrPtr CompanyName		= "GeoCell";
    const StrPtr ProjectName		= "VulkanSDL exp";

    const StrPtr DataPath		    = "data/";

    const int DefaultWindowWidth	= 1900;
    const int DefaultWindowHeight	= 1000;

    const int MaxSaneScreenWidth	= 3800 * 1;		// 8K x 2 values. Simply sanity-
    const int MaxSaneScreenHeight	= 1200 * 1;		//	check on pixel-related ranges.

    VkClearValue DefaultClearColor = { { { 0.1f, 0.1f, 0.1f, 1.0f } } };

	const bool SupportStereo3D		= false;

	StrPtr	getExePath() const	 {	return exePath; }

	void	setExePath(StrPtr p) {	if (exePath == nullptr)
		/* Only allow set once! */		exePath = p;		}

private:
    StrPtr	exePath					= nullptr;

public:
	AppSettings	 Settings;			// singleton, instantiate LAST: uses
									//		constants initialized above
} AppConstants;


#ifdef extern
  #undef extern
#endif

#endif // AppConstants_h


/*       1         2         3         4         5         6         7         8         9        10        11        12
123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
*/
