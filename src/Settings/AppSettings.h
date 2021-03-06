//
// AppSettings.h
//	Application Properties/Attributes
//
// To encapsulate:  (design/needs/choices such as:)
//	- "Delivery" of universal (app-wide) values that may be needed anywhere.
//	- Security (risk of someone changing) vs. ease-of-access/speed-of-access.
//	- Persist of Attributes:
//	  - First of all, should settings persist? or always assume default values.
//	  - How?  File or hard-coded (testing) data or over network/cloud (*).
//	  - Format?  JSON (probably ever "only choice" but in theory, encapsulated).
//	  - Specific choice of which JSON Parser (i.e. if we decide to go with
//		a different one later, only this class is impacted).
//	  - Encryption layer possible (for e.g. a password).
//	  - (*) - Specific HTTP library chosen.
// There must be strong differentiation between Platform variables and App settings.
//	The latter may include default values that are app-arbitrary, but the former
//	are operational.  However, this class can still be used to "deliver" those.
// Interfaces with either File or Network (HTTP) component to abstract
//	destination for storage/retrieval.
//
// Created 6/10/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef AppSettings_h
#define AppSettings_h

#include <string>
//#include "jsonConvert_c.h"
#include <iostream>
#include <iomanip>
#include "nlohmann/json.hpp"
//#include "json.hpp"
using json = nlohmann::json;
using value_t = nlohmann::detail::value_t;


class AppSettings
{
public:
	AppSettings();

	void Save();
	void Retrieve();

    std::string filePath;

	bool isInitialized = false;

	// APP/GENERAL SETTINGS

	bool isDebugLogToFile = true;

	// WINDOWING-SYSTEM/GRAPHICS STARTUP SETTINGS

	int startingWindowWidth;
	int startingWindowHeight;
	int startingWindowX;
	int startingWindowY;

    json jsonSettingsTemp;

    // AppSettings atomics

    inline void from_json(const json& jsonAppSettings, AppSettings& appSettings)
    {
        appSettings.startingWindowWidth	 = jsonAppSettings["startingWindowWidth"];
        appSettings.startingWindowHeight = jsonAppSettings["startingWindowHeight"];
        appSettings.startingWindowX		 = jsonAppSettings["startingWindowX"];
        appSettings.startingWindowY		 = jsonAppSettings["startingWindowY"];
    }

    inline void to_json(json& jsonAppSettings, const AppSettings& settings)
    {
        jsonAppSettings = json{
            { "startingWindowWidth",  settings.startingWindowWidth },
            { "startingWindowHeight", settings.startingWindowHeight },
            { "startingWindowX", settings.startingWindowX },
            { "startingWindowY", settings.startingWindowY }
        };
    }





};

#endif	// AppSettings_h
