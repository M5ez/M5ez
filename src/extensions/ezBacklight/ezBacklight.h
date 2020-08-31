#pragma once

// Coupling:
// Indicate activity to the ezBacklight extension by calling ez.extensionControl("ezBacklight", EXTENSION_CONTROL_PING, nullptr)

class ezBacklight {
	public:
		static bool control(uint8_t command, void* /* user */);
		static void begin();
		static void menu();
		static void inactivity(uint8_t half_minutes);
		static void activity();
		static uint16_t loop();
	private:
		static uint8_t _brightness;
		static uint8_t _inactivity;
		static uint32_t _last_activity;
		static bool _backlight_off;
};
