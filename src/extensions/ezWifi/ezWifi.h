#pragma once

#include <WiFi.h>

// Comment out the line below to disable WPS.
#define M5EZ_WPS

enum WifiState_t {
	EZWIFI_NOT_INIT,
	EZWIFI_WAITING,
	EZWIFI_IDLE,
	EZWIFI_SCANNING,
	EZWIFI_CONNECTING,
	EZWIFI_AUTOCONNECT_DISABLED
};

struct WifiNetwork_t {
	String SSID;
	String key;
};

class ezWifi {
	public:
		static bool control(uint8_t command, void* /* user */);
		static void begin();
		static void menu();
		static void add(String ssid, String key);
		static bool remove(int8_t index);
		static bool remove(String ssid);
		static int8_t indexForSSID(String ssid);
		static void readFlash();
		static void writeFlash();
		static uint16_t loop();
		static bool update(String url, const char* root_cert, ezProgressBar* pb = NULL);
		static String updateError();
		static bool autoConnect;
		static std::vector<WifiNetwork_t> networks;
	private:
		static WifiState_t _state;
		static uint8_t _current_from_scan;
		static uint32_t _wait_until, _widget_time;
		static void _drawWidget(uint16_t x, uint16_t w);
		static bool _onOff(ezMenu* callingMenu);
		static void _manageAutoconnects();
		static bool _autoconnectSelected(ezMenu* callingMenu);
		static void _askAdd();
		static bool _connection(ezMenu* callingMenu);
		static void _update_progress(int done, int total);
		static String _update_err2str(uint8_t _error);
		static ezProgressBar* _update_progressbar;
		static String _update_error;
		#ifdef M5EZ_WPS
			static void _WPShelper(WiFiEvent_t event, system_event_info_t info);
			static WiFiEvent_t _WPS_event;
			static String _WPS_pin;
			static bool _WPS_new_event;
		#endif
};
