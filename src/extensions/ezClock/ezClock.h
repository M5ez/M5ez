#pragma once
#include <ezTime.h>

class ezClock {
	public:
		static bool control(uint8_t command, void* /* reserved */);
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
