#pragma once
#include <ezTime.h>

// Coupling:
// ezClock (actually ezTime) exposes one function that M5ez calls from yield: events().
// It's implemented with control(EXTENSION_CONTROL_CLOCK_EVENTS, nullptr)

#define EXTENSION_CONTROL_CLOCK_EVENTS	11

class ezClock {
	public:
		static bool control(uint8_t command, void* /* user */);
		static void begin();
		static void restart();
		static void menu();
		static uint16_t loop();
		static void draw(uint16_t x, uint16_t w);
		static bool waitForSync(const uint16_t timeout = 0);
		static Timezone tz;
	private:
		static void _writePrefs();
		static bool _on;
		static String _timezone;
		static bool _clock12;
		static bool _am_pm;
		static String _datetime;
		static bool _starting;
};
