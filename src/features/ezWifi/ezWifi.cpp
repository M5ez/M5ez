#include <Preferences.h>
extern "C" {
	#include "esp_wifi.h"
	#include "esp_wps.h"
}
#include <WiFiClientSecure.h>		// For ez.update
#include <Update.h>
#include "../../M5ez.h"
#include "ezWifi.h"

// Have the autoconnect logic print debug messages on the serial port
// #define M5EZ_WIFI_DEBUG

WifiState_t ezWifi::_state;
uint8_t ezWifi::_current_from_scan;
uint32_t ezWifi::_wait_until;
uint32_t ezWifi::_widget_time;
std::vector<WifiNetwork_t> ezWifi::networks;
ezProgressBar* ezWifi::_update_progressbar;
String ezWifi::_update_error;
bool ezWifi::autoConnect;
#ifdef M5EZ_WPS
	WiFiEvent_t ezWifi::_WPS_event;
	String ezWifi::_WPS_pin;
	bool ezWifi::_WPS_new_event;
#endif


bool ezWifi::entry(uint8_t command, void* /* user */) {
	switch(command) {
		case FEATURE_MSG_PING:
			return true;
		case FEATURE_MSG_START:
			begin();
			return true;
	}
	return false;
}


void ezWifi::begin() {
	#ifdef M5EZ_WIFI_DEBUG
		Serial.println("EZWIFI: Initialising");
	#endif
	WiFi.mode(WIFI_MODE_STA);
	WiFi.setAutoConnect(false);		// We have our own multi-AP version of this
	WiFi.setAutoReconnect(false);	// So we turn off the ESP32's versions
	WiFi.setHostname("M5Stack");
	readFlash();
	_state = EZWIFI_IDLE;
	const uint8_t cutoffs[] = { 0, 20, 40, 70 };
	ez.settings.menuObj.addItem("Wifi settings", menu);
	ez.header.insert(RIGHTMOST, "wifi", sizeof(cutoffs) * (ez.theme->signal_bar_width + ez.theme->signal_bar_gap) + 2 * ez.theme->header_hmargin, _drawWidget);
	// For handling issue #50, when initial connection attempt fails in this specific mode but will succeed if tried again.
	WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
		if(WIFI_REASON_ASSOC_FAIL == info.disconnected.reason) {
		#ifdef M5EZ_WIFI_DEBUG
			Serial.println("EZWIFI: Special case: Disconnect w/ ASSOC_FAIL. Setting _state to EZWIFI_SCANNING;");
		#endif
		_state = EZWIFI_SCANNING;
	}
	}, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
	ez.addEvent(loop);
}

void ezWifi::_drawWidget(uint16_t x, uint16_t w) {
	const uint8_t cutoffs[] = { 0, 20, 40, 70 };
	uint8_t max_bars = sizeof(cutoffs);
	uint8_t bars;
	uint16_t left_offset = x + ez.theme->header_hmargin;
	bars = 0;
	if (WiFi.isConnected()) {
		uint8_t signal = map(100 + WiFi.RSSI(), 5, 90, 0, 100);
		for (uint8_t n = 0; n < sizeof(cutoffs); n++) {				// Determine how many bars signal is.
			if (signal >= cutoffs[n]) bars = n + 1;
		}
	}
	uint8_t top = ez.theme->header_height / 10;
	uint8_t max_len = ez.theme->header_height * 0.8;
	uint8_t this_len;
	for (uint8_t n = 0; n < max_bars; n++) {
		this_len = ((float) (n + 1) / max_bars) * max_len;
		m5.lcd.fillRect(left_offset + n * (ez.theme->signal_bar_width + ez.theme->signal_bar_gap), top + max_len - this_len, ez.theme->signal_bar_width, this_len, (n + 1 <= bars ? ez.theme->header_fgcolor : ez.theme->header_bgcolor) );
	}
}

void ezWifi::add(String ssid, String key){
	WifiNetwork_t new_net;
	new_net.SSID = ssid;
	new_net.key = key;
	networks.push_back(new_net);
}

bool ezWifi::remove(int8_t index) {
	if (index < 0 || index >= networks.size()) return false;
	networks.erase(networks.begin() + index);
	return true;
}

bool ezWifi::remove(String ssid) { return remove(indexForSSID(ssid)); }

int8_t ezWifi::indexForSSID(String ssid) {
	for (uint8_t n = 0; n < networks.size(); n++) {
		if (networks[n].SSID == ssid) return n;
	}
	return -1;
}

void ezWifi::readFlash() {
	Preferences prefs;
	networks.clear();
	prefs.begin("M5ez", true);	// true: read-only
	autoConnect = prefs.getBool("autoconnect_on", true);
	#ifdef M5EZ_WIFI_DEBUG
		Serial.println("wifiReadFlash: Autoconnect is " + (String)(autoConnect ? "ON" : "OFF"));
	#endif
	WifiNetwork_t new_net;
	String idx;
	uint8_t index = 1;
	while (true) {
		idx = "SSID" + (String)index;
		String ssid = prefs.getString(idx.c_str(), "");
		idx = "key" + (String)index;
		String key = prefs.getString(idx.c_str(), "");
		if (ssid != "") {
			new_net.SSID = ssid;
			new_net.key = key;
			networks.push_back(new_net);
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("wifiReadFlash: Read ssid:" + ssid + " key:" + key);
			#endif
			index++;
		} else {
			break;
		}
	}
	prefs.end();
}

void ezWifi::writeFlash() {
	Preferences prefs;
	String idx;
	uint8_t n = 1;
	prefs.begin("M5ez", false);
	// Remove unknown number of items from NVS, sequentially named SSID1 to SSIDN, and key1 to keyN where N
	// is the total number of WiFi Networks stored (which may be different than networks.size() at this point.)
	while (true) {
		idx = "SSID" + (String)n;
		if(!prefs.remove(idx.c_str())) break;
		idx = "key" + (String)n;
		prefs.remove(idx.c_str());
		n++;
	}
	prefs.putBool("autoconnect_on", autoConnect);
	#ifdef M5EZ_WIFI_DEBUG
		Serial.println("wifiWriteFlash: Autoconnect is " + (String)(autoConnect ? "ON" : "OFF"));
	#endif
	for (n = 0; n < networks.size(); n++) {
		idx = "SSID" + (String)(n + 1);
		prefs.putString(idx.c_str(), networks[n].SSID);
		if (networks[n].key != "") {
			idx = "key" + (String)(n + 1);
			prefs.putString(idx.c_str(), networks[n].key);
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("wifiWriteFlash: Wrote ssid:" + networks[n].SSID + " key:" + networks[n].key);
			#endif
		}
	}
	prefs.end();
}

void ezWifi::menu() {
	_state = EZWIFI_AUTOCONNECT_DISABLED;
	#ifdef M5EZ_WIFI_DEBUG
		Serial.println("EZWIFI: Disabling autoconnect while in Wifi menu.");
	#endif
	ezMenu wifimain ("Wifi settings");
	wifimain.txtSmall();
	wifimain.addItem("onoff | Autoconnect\t" + (String)(autoConnect ? "ON" : "OFF"), NULL, _onOff);
	wifimain.addItem("connection | " + (String)(WiFi.isConnected() ? "Connected: " + WiFi.SSID() : "Join a network"), NULL, _connection);
	wifimain.addItem("Manage autoconnects", _manageAutoconnects);
	wifimain.buttons("up#Back#select##down#");
	wifimain.run();
	_state = EZWIFI_IDLE;
	#ifdef M5EZ_WIFI_DEBUG
		Serial.println("EZWIFI: Enabling autoconnect exiting Wifi menu.");
	#endif
}

bool ezWifi::_onOff(ezMenu* callingMenu) {
	autoConnect = !autoConnect;
	callingMenu->setCaption("onoff", "Autoconnect\t" + (String)(autoConnect ? "ON" : "OFF"));
	writeFlash();
	return true;
}

void ezWifi::_manageAutoconnects() {
	ezMenu autoconnect("Managing autoconnects");
	if (!networks.size()) {
		ez.msgBox("No autoconnects", "You have no saved autoconnect networks.", "OK");
		return;
	}
	for (uint8_t n = 0; n < networks.size(); n++) {
		autoconnect.addItem(networks[n].SSID, NULL, _autoconnectSelected);
	}
	autoconnect.txtSmall();
	autoconnect.buttons("up#Back#Forget##down#");
	autoconnect.run();
}

bool ezWifi::_autoconnectSelected(ezMenu* callingMenu) {
	if (callingMenu->pickButton() == "Forget") {
		if (ez.msgBox("Forgetting wifi network", "Are you sure you want | to forget wifi network | " + callingMenu->pickName() + " ?", "Yes##No") == "Yes") {
			remove(callingMenu->pick() - 1);
			callingMenu->deleteItem(callingMenu->pick());
			writeFlash();
		}
	}
	return false;
}

bool ezWifi::_connection(ezMenu* callingMenu) {
	if (WiFi.isConnected()) {
		const uint8_t tab = 140;
		ez.screen.clear();
		ez.header.show("Current wifi connection");
		ez.canvas.font(&FreeSans9pt7b);
		ez.canvas.lmargin(10);
		ez.canvas.y(ez.canvas.top() + 5);
		ez.canvas.print("SSID:"); ez.canvas.x(tab); ez.canvas.println(WiFi.SSID());
		ez.canvas.print("Key:"); ez.canvas.x(tab); ez.canvas.println(WiFi.psk());
		ez.canvas.print("My IP:"); ez.canvas.x(tab); ez.canvas.println(WiFi.localIP().toString());
		ez.canvas.print("My MAC:"); ez.canvas.x(tab); ez.canvas.println(WiFi.macAddress());
		ez.canvas.print("My hostname:"); ez.canvas.x(tab); ez.canvas.println(WiFi.getHostname());
		ez.canvas.print("Router IP:"); ez.canvas.x(tab); ez.canvas.println(WiFi.gatewayIP().toString());
		ez.canvas.print("Router BSSID:"); ez.canvas.x(tab); ez.canvas.println(WiFi.BSSIDstr());
		ez.canvas.print("DNS IP:"); ez.canvas.x(tab); ez.canvas.println(WiFi.dnsIP(0).toString());
		if (WiFi.dnsIP(1)) { ez.canvas.x(tab); ez.canvas.println(WiFi.dnsIP(1).toString()); }
		String pressed = ez.buttons.wait("Back#Disconnect#");
		if (pressed == "Back") return true;
		if (pressed == "Disconnect") {
			WiFi.disconnect();
			while(WiFi.isConnected()) {}
		}

	} else {

		String SSID = "", key = "";
		ezMenu joinmenu("Joining a network");
		joinmenu.txtSmall();
		joinmenu.addItem("Scan and join");
		joinmenu.addItem("SmartConfig");
		#ifdef M5EZ_WPS
			joinmenu.addItem("WPS Button");
			joinmenu.addItem("WPS Pin Code");
		#endif
		joinmenu.buttons("up#Back#select##down#");
		joinmenu.runOnce();

		if (joinmenu.pickName() == "Scan and join") {
			ez.msgBox("WiFi setup menu", "Scanning ...", "");
			WiFi.disconnect();
			WiFi._setStatus(WL_IDLE_STATUS);
			delay(100);
			int16_t n = WiFi.scanNetworks();
			if (n == 0) {
				ez.msgBox("WiFi setup menu", "No networks found", "OK");
			} else {
				ezMenu networks("Select your netork");
				networks.txtSmall();
				for (uint16_t i = 0; i < n; ++i) {
					// No duplicates (multiple BSSIDs on same SSID)
					// because we're only picking an SSID here
					if (!networks.getItemNum(WiFi.SSID(i))) {
						networks.addItem(WiFi.SSID(i));
					}
				}
				networks.buttons("up#Back#select##down#");
				if (networks.runOnce()) {
					SSID = networks.pickName();
					if ( WiFi.encryptionType(networks.pick() - 1) == WIFI_AUTH_OPEN) {
						WiFi.mode(WIFI_MODE_STA);
						WiFi.begin(SSID.c_str());
					} else {
						key = ez.textInput("Enter wifi password");
						WiFi.mode(WIFI_MODE_STA);
						WiFi.begin(SSID.c_str(), key.c_str());
					}
					ez.msgBox("WiFi setup menu", "Connecting ...", "Abort", false);
					String button;
					int16_t status;
					while (button = ez.buttons.poll()) {
						if (button == "Abort") {
							WiFi.disconnect();
							break;
						}
						status = WiFi.status();
						if (status == WL_CONNECTED) {
							break;
						}
						if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
							ez.msgBox("WiFi setup menu", "Connect failed. | | (Wrong password?)", "OK");
							break;
						}
					}
				}
				WiFi.scanDelete();
			}
		}

		if (joinmenu.pickName() == "SmartConfig") {
			ez.msgBox("SmartConfig setup", "Waiting for SmartConfig", "Abort", false);
			WiFi.mode(WIFI_MODE_STA);
			WiFi.beginSmartConfig();
			bool done_already = false;
			while (!WiFi.isConnected()) {
				if (ez.buttons.poll() == "Abort") {
					WiFi.stopSmartConfig();
					break;
				}
				if (WiFi.smartConfigDone() && !done_already) {
					ez.msgBox("SmartConfig setup", "SmartConfig received | Connecting ...", "Abort", false);
					done_already = true;
				}
			}
		}

		#ifdef M5EZ_WPS
			if (joinmenu.pickName().substring(0,3) == "WPS") {
				ez.msgBox("WPS setup", "Waiting for WPS", "Abort", false);
				WiFi.mode(WIFI_MODE_STA);
				static esp_wps_config_t config;
				config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
				strcpy(config.factory_info.manufacturer, "ESPRESSIF");
				strcpy(config.factory_info.model_number, "ESP32");
				strcpy(config.factory_info.model_name, "ESPRESSIF IOT");
				strcpy(config.factory_info.device_name, "ESP STATION");
				if (joinmenu.pickName() == "WPS Button") {
					config.wps_type = WPS_TYPE_PBC;
				} else {
					config.wps_type = WPS_TYPE_PIN;
				}
				WiFi.onEvent(_WPShelper);
				esp_wifi_wps_enable(&config);
				esp_wifi_wps_start(0);

				_WPS_new_event = false;
				while (!WiFi.isConnected()) {
					if (ez.buttons.poll() == "Abort") {
						esp_wifi_wps_disable();
						break;
					}
					if (_WPS_new_event) {
						switch(_WPS_event) {
							case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
								ez.msgBox("WPS setup", "WPS successful | Connecting ...", "Abort", false);
								esp_wifi_wps_disable();
								delay(10);
								WiFi.begin();
								break;
							case SYSTEM_EVENT_STA_WPS_ER_FAILED:
							case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
								ez.msgBox("WPS setup", "WPS failed or timed out | Retrying ...", "Abort", false);
								esp_wifi_wps_disable();
								esp_wifi_wps_enable(&config);
								esp_wifi_wps_start(0);
								break;
							case SYSTEM_EVENT_STA_WPS_ER_PIN:
								ez.msgBox("WPS setup", "WPS PIN: " + _WPS_pin, "Abort", false);
								break;
							default:
								break;
						}
						_WPS_new_event = false;
					}
				}
			}
		#endif

		if (WiFi.isConnected()) _askAdd();
	}
	callingMenu->setCaption("connection", (String)(WiFi.isConnected() ? "Connected: " + WiFi.SSID() : "Join a network"));
	return true;
}

#ifdef M5EZ_WPS
	void ezWifi::_WPShelper(WiFiEvent_t event, system_event_info_t info) {
		_WPS_event = event;
		_WPS_new_event = true;
		if (event == SYSTEM_EVENT_STA_WPS_ER_PIN) {
			char wps_pin[9];
			for (int8_t i = 0; i < 8; i++) {
				wps_pin[i] = info.sta_er_pin.pin_code[i];
			}
			wps_pin[8] = '\0';
			_WPS_pin = String(wps_pin);
		}
	}
#endif

void ezWifi::_askAdd() {
	for (uint8_t n = 0; n < networks.size(); n++) {
		if (networks[n].SSID == WiFi.SSID()) return;
	}
	if (ez.msgBox("Wifi settings", "Save this network | to your autoconnects?", "no##yes") == "yes") {
		add(WiFi.SSID(), WiFi.psk());
		writeFlash();
	}
}

uint16_t ezWifi::loop() {
	if (millis() > _widget_time + ez.theme->signal_interval) {
		ez.header.draw("wifi");
		_widget_time = millis();
	}
	if (WiFi.isConnected() && _state != EZWIFI_AUTOCONNECT_DISABLED && _state != EZWIFI_IDLE) {
		_state = EZWIFI_IDLE;
		#ifdef M5EZ_WIFI_DEBUG
			Serial.println("EZWIFI: Connected, returning to IDLE state");
		#endif
	}
	if (!autoConnect || WiFi.isConnected() || networks.size() == 0) return 250;
	int8_t scanresult;
	switch(_state) {
		case EZWIFI_WAITING:
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("EZWIFI: State Machine: _state = EZWIFI_WAITING");
			#endif
			if (millis() < _wait_until) return 250;
			// intentional fall-through
		case EZWIFI_IDLE:
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("EZWIFI: State Machine: _state = EZWIFI_IDLE");
				Serial.println("EZWIFI: Starting scan");
			#endif
			WiFi.mode(WIFI_MODE_STA);
			WiFi.scanNetworks(true);
			_current_from_scan = 0;
			_state = EZWIFI_SCANNING;
			_wait_until = millis() + 10000;
			break;
		case EZWIFI_SCANNING:
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("EZWIFI: State Machine: _state = EZWIFI_SCANNING");
			#endif
			scanresult = WiFi.scanComplete();
			switch(scanresult) {
				case WIFI_SCAN_RUNNING:
					break;
				case WIFI_SCAN_FAILED:
					#ifdef M5EZ_WIFI_DEBUG
						Serial.println("EZWIFI: Scan failed");
					#endif
					_state = EZWIFI_WAITING;
					_wait_until = millis() + 60000;
					WiFi.scanDelete();
					return 250;
				default:
					#ifdef M5EZ_WIFI_DEBUG
						Serial.println("EZWIFI: Scan got " + (String)scanresult + " networks");
					#endif
					for (uint8_t n = _current_from_scan; n < scanresult; n++) {
						for (uint8_t m = 0; m < networks.size(); m++) {
							String ssid = networks[m].SSID;
							String key = networks[m].key;
							if (ssid == WiFi.SSID(n)) {
								#ifdef M5EZ_WIFI_DEBUG
									Serial.println("EZWIFI: Match: " + WiFi.SSID(n) + ", connecting...");
								#endif
								WiFi.mode(WIFI_MODE_STA);
								WiFi.begin(ssid.c_str(), key.c_str());
								_state = EZWIFI_CONNECTING;
								_wait_until = millis() + 7000;
								return 250;
							}
						}
					}
					#ifdef M5EZ_WIFI_DEBUG
						Serial.println("EZWIFI: No (further) matches, waiting...");
					#endif
					_state = EZWIFI_WAITING;
					_wait_until = millis() + 60000;
					WiFi.scanDelete();
				//
			}
		case EZWIFI_CONNECTING:
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("EZWIFI: State Machine: _state = EZWIFI_CONNECTING");
			#endif
			if (millis() > _wait_until) {
				#ifdef M5EZ_WIFI_DEBUG
					Serial.println("EZWIFI: Connect timed out...");
				#endif
				WiFi.disconnect();
				_current_from_scan++;
				_state = EZWIFI_SCANNING;
			}
			break;
		case EZWIFI_AUTOCONNECT_DISABLED:
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("EZWIFI: State Machine: _state = EZWIFI_AUTOCONNECT_DISABLED");
			#endif
			break;
		default:
			#ifdef M5EZ_WIFI_DEBUG
				Serial.println("EZWIFI: State Machine: default case! _state = " + String(_state));
			#endif
			break;
	}
	return 250;
}

bool ezWifi::update(String url, const char* root_cert, ezProgressBar* pb /* = NULL */) {

	_update_progressbar = pb;

	if (!WiFi.isConnected()) {
		_update_error = "No WiFi connection.";
		return false;
	}

	if (!url.startsWith("https://")) {
		_update_error = "URL must start with 'https://'";
		return false;
	}

	url = url.substring(8);

	String host, file;
	uint16_t port;
	int16_t first_slash_pos = url.indexOf("/");
	if (first_slash_pos == -1) {
		host = url;
		file = "/";
	} else {
		host = url.substring(0, first_slash_pos);
		file = url.substring(first_slash_pos);
	}
	int16_t colon = host.indexOf(":");

	if (colon == -1) {
		port = 443;
	} else {
		host = host.substring(0, colon);
		port = host.substring(colon + 1).toInt();
	}

	WiFiClientSecure client;
	client.setTimeout(20000);
	client.setCACert(root_cert);

	int contentLength = 0;

	if (!client.connect(host.c_str(), port)) {
		_update_error = "Connection to " + String(host) + " failed.";
		return false;
	}

	client.print(String("GET ") + file + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"Cache-Control: no-cache\r\n" +
		"Connection: close\r\n\r\n");

	unsigned long timeout = millis();
	while (!client.available()) {
		if (millis() - timeout > 10000) {
			_update_error = "Client Timeout";
			client.stop();
			return false;
		}
	}

	// Process header
	while (client.available()) {
		String line = client.readStringUntil('\n');
		line.trim();
		if (!line.length()) break; // empty line, assume headers done

		if (line.startsWith("HTTP/1.1")) {
			String http_response = line.substring(line.indexOf(" ") + 1);
			http_response.trim();
			if (http_response.indexOf("200") == -1) {
				_update_error = "Got response: \"" + http_response + "\", must be 200";
				return false;
			}
		}

		if (line.startsWith("Content-Length: ")) {
			contentLength = atoi(line.substring(  line.indexOf(":") + 1  ).c_str());
			if (contentLength <= 0) {
				_update_error = "Content-Length zero";
				return false;
			}
		}

		if (line.startsWith("Content-Type: ")) {
			String contentType = line.substring(line.indexOf(":") + 1);
			contentType.trim();
			if (contentType != "application/octet-stream") {
				_update_error = "Content-Type must be \"application/octet-stream\", got \"" + contentType + "\"";
				return false;
			}
		}
	}

	// Process payload
	Update.onProgress(_update_progress);

	if (!Update.begin(contentLength)) {
		_update_error = "Not enough space to begin OTA";
		client.flush();
		return false;
	}

	size_t written = Update.writeStream(client);

	if (!Update.end()) {
		_update_error = "Error: " + String(_update_err2str(Update.getError())) + " | (after " + String(written) + " of " + String(contentLength) + " bytes)";
		return false;
	}

	if (!Update.isFinished()) {
		_update_error = "Update not finished. Something went wrong.";
		return false;
	}

	return true;

}

void ezWifi::_update_progress(int done, int total) {
	if (ez.buttons.poll() != "") {
		Update.abort();
	} else {
		if (total && _update_progressbar != NULL) {
			_update_progressbar->value((done * 100) / total);
		}
	}
}

String ezWifi::updateError() { return _update_error; }

// Stupid Updater library only wants to print readable errors to a Stream object, 
// so we just copied its _err2str function. Bleh...
String ezWifi::_update_err2str(uint8_t _error) {
	if(_error == UPDATE_ERROR_OK){
		return ("No Error");
	} else if(_error == UPDATE_ERROR_WRITE){
		return ("Flash Write Failed");
	} else if(_error == UPDATE_ERROR_ERASE){
		return ("Flash Erase Failed");
	} else if(_error == UPDATE_ERROR_READ){
		return ("Flash Read Failed");
	} else if(_error == UPDATE_ERROR_SPACE){
		return ("Not Enough Space");
	} else if(_error == UPDATE_ERROR_SIZE){
		return ("Bad Size Given");
	} else if(_error == UPDATE_ERROR_STREAM){
		return ("Stream Read Timeout");
	} else if(_error == UPDATE_ERROR_MD5){
		return ("MD5 Check Failed");
	} else if(_error == UPDATE_ERROR_MAGIC_BYTE){
		return ("Wrong Magic Byte");
	} else if(_error == UPDATE_ERROR_ACTIVATE){
		return ("Could Not Activate The Firmware");
	} else if(_error == UPDATE_ERROR_NO_PARTITION){
		return ("Partition Could Not be Found");
	} else if(_error == UPDATE_ERROR_BAD_ARGUMENT){
		return ("Bad Argument");
	} else if(_error == UPDATE_ERROR_ABORT){
		return ("Aborted");
	}
	return ("UNKNOWN");
}
