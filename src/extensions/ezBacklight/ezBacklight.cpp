#include <Preferences.h>
#include "../../M5ez.h"
#include "ezBacklight.h"

#define NEVER		0
#define USER_SET	255


uint8_t ezBacklight::_brightness;
uint8_t ezBacklight::_inactivity;
uint32_t ezBacklight::_last_activity;
bool ezBacklight::_backlight_off = false;


bool ezBacklight::control(uint8_t command, void* /* user */) {
	switch(command) {
		case EXTENSION_CONTROL_PING:
			activity();
			return true;
		case EXTENSION_CONTROL_START:
			begin();
			return true;
	}
	return false;
}

void ezBacklight::begin() {
	ez.addEvent(ezBacklight::loop);
	ez.settings.menuObj.addItem("Backlight settings", ezBacklight::menu);
	Preferences prefs;
	prefs.begin("M5ez", true);	// read-only
	_brightness = prefs.getUChar("brightness", 128);
	_inactivity = prefs.getUChar("inactivity", NEVER);
	prefs.end();
	m5.lcd.setBrightness(_brightness);
}

void ezBacklight::menu() {
	uint8_t start_brightness = _brightness;
	uint8_t start_inactivity = _inactivity;
	ezMenu blmenu("Backlight settings");
	blmenu.txtSmall();
	blmenu.buttons("up#Back#select##down#");
	blmenu.addItem("Backlight brightness");
	blmenu.addItem("Inactivity timeout");
	while(true) {
		switch (blmenu.runOnce()) {
			case 1:	
				{
					ezProgressBar bl ("Backlight brightness", "Set brightness", "left#ok#right");
					while (true) {
						String b = ez.buttons.poll();
						if (b == "right" && _brightness <= 240) _brightness += 16;
						if (!_brightness) _brightness = 255;
						if (b == "left" && _brightness > 16) _brightness -= 16;
						if (_brightness == 239) _brightness = 240;
						bl.value((float)(_brightness / 2.55));
						m5.lcd.setBrightness(_brightness);
						if (b == "ok") break;
					}
				}
				break;
			case 2:
				{
					String disp_val;
					while (true) {
						if (!_inactivity) {
							disp_val = "Backlight will not turn off";
						} else if (_inactivity == 1) {
							disp_val = "Backlight will turn off after 30 seconds of inactivity";
						} else if (_inactivity == 2) {
							disp_val = "Backlight will turn off after a minute of inactivity";
						} else {
							disp_val = "Backlight will turn off after " + String(_inactivity / 2) + ((_inactivity % 2) ? ".5 " : "") + " minutes of inactivity";
						}
						ez.msgBox("Inactivity timeout", disp_val, "-#--#ok##+#++", false);
						String b = ez.buttons.wait();
						if (b == "-" && _inactivity) _inactivity--;
						if (b == "+" && _inactivity < 254) _inactivity++;
						if (b == "--") {
							if (_inactivity < 20) {
								_inactivity = 0;
							} else {
								_inactivity -= 20;
							}
						}
						if (b == "++") {
							if (_inactivity > 234) {
								_inactivity = 254;
							} else {
								_inactivity += 20;
							}
						}
						if (b == "ok") break;
					}
				}
				break;
			case 0:
				if (_brightness != start_brightness || _inactivity != start_inactivity) {
					Preferences prefs;
					prefs.begin("M5ez");
					prefs.putUChar("brightness", _brightness);
					prefs.putUChar("inactivity", _inactivity);
					prefs.end();
				}
				return;
		}
	}
}

void ezBacklight::inactivity(uint8_t half_minutes) {
	if (half_minutes == USER_SET) {
		Preferences prefs;
		prefs.begin("M5ez", true);
		_inactivity = prefs.getUShort("inactivity", 0);
		prefs.end();
	} else {
		_inactivity = half_minutes;
	}
}

void ezBacklight::activity() {
	_last_activity = millis();
}

uint16_t ezBacklight::loop() {
	if (!_backlight_off && _inactivity) {
		if (millis() > _last_activity + 30000 * _inactivity) {
			_backlight_off = true;
			m5.lcd.setBrightness(0);
			while (true) {
				if (m5.BtnA.wasPressed() || m5.BtnB.wasPressed() || m5.BtnC.wasPressed()) break;
				ez.yield();
				delay(10);
			}
			ez.buttons.releaseWait();	// Make sure the key pressed to wake display gets ignored
			m5.lcd.setBrightness(_brightness);
			activity();
			_backlight_off = false;
		}
	}
	return 1000;
}
