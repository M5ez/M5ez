#pragma once
#include <M5ez.h>

class ScreenShotExtension {
	public:
		static bool control(uint8_t command, void* /* user */);
		static void begin();
		static void menu();
		static uint16_t loop();
	private:
		static void _readPrefs();
		static void _writePrefs();

		static const String _triggers[6];       // None | A | B | C | Shake | ezNow
		static const String _extensions[2];     // jpg | png
		static const String _fileSystems[2];    // SPIFFS | SD
		static const String _name;              // "Screen Shot"
		static uint8_t _trigger;                // index into _triggers
		static uint8_t _extension;              // index into _extensions
		static uint8_t _fs;                     // 0 for SPIFFS, 1 for SD, 2 for Serial, 3 for Serial2
		static String _fileName;                // May include a path, use %n format expression
};
