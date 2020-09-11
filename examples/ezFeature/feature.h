#pragma once
#include <M5ez.h>

#define FEATURE_NAME	"Feature"

// Feature extensions rely on several static methods, this means some approaches you may
// be tempted to try are not available. You cannot create a subclass of Feature with
// specialized static functions; static functions cannot be virtual.
// You cannot initialize static data in the class declaration, you must initialize it separately in the .cpp file. (See top of feature.cpp).
// Instead, copy this file and rename the class.
// Eliminate or rename _datum, which is provided as an illustration.
// You must implement entry(). It is called with several different command parameters. See feature.cpp for details.
// Implement menu() if you have UI for adjusting your settings. You must add yourself to the menu when called.
// Implement loop() if you would like to be called periodically. You must add yourself as an event when called.
// _readPrefs() and _writePrefs() are conventional names for functions to load and save settings to NVS. An example is given. They are not required.
// When provided, call _readPrefs() when loaded, _writePrefs() whenever a settings is changed.
// The extensions's name will be needed in several contexts; best to have a single _name member variable to provide it.
// Once added, the public interface of your static class is visible globally via its namespace: "Feature::" in this case.
// You could manually call the menu function from you main program by invoking "Feature::menu()". The same goes for any public static methods you add.
// Consider providing public data accessors for your private settings data, like "static uint16_t get_altitude()".
//
class Feature {
	public:
		static bool entry(uint8_t command, void* data);
		static void menu();
		static uint16_t loop();
	private:
		static const String _name;
		static void _readPrefs();
		static void _writePrefs();
		static bool _datum; 	// Provided as an example for loading/saving/displaying settings
};
