#pragma once

class ezBattery {
	public:
		static bool control(uint8_t command, void* /* reserved */);
		static void begin();
		static void menu();
		static uint16_t loop();
	private:
		static void _readFlash();
		static void _writeFlash();
		static uint8_t _getTransformedBatteryLevel();
		static uint32_t _getBatteryBarColor(uint8_t batteryLevel);
		static bool _on;
		static void _refresh();
		static void _drawWidget(uint16_t x, uint16_t w);
};
