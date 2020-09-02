#include <Preferences.h>
#include "ScreenShotFeature.h"

const String ScreenShotFeature::_name = "Screen Shot";
const String ScreenShotFeature::_triggers[] = { "None", "A", "B", "C", "Shake", "ezNow" };
const String ScreenShotFeature::_extensions[] = { "jpg", "png" };
const String ScreenShotFeature::_fileSystems[] = { "SPIFFS", "SD" };
uint8_t ScreenShotFeature::_trigger;      // index into _triggers
uint8_t ScreenShotFeature::_extension;    // index into _extensions
uint8_t ScreenShotFeature::_fs;           // 0 for SPIFFS, 1 for SD, 2 for Serial, 3 for Serial2
String ScreenShotFeature::_fileName;      // May include a path, use %n format expression


bool ScreenShotFeature::entry(uint8_t command, void* /* user */) {
	switch(command) {
		case FEATURE_MSG_PING:
			return true;
		case FEATURE_MSG_START:
			begin();
			return true;
		case FEATURE_MSG_STOP:
			_trigger = 0;
			return true;
		case FEATURE_MSG_QUERY_ENABLED:
			return _trigger != 0;
	}

	return false;
}

void ScreenShotFeature::begin() {
	_readPrefs();
	ez.addEvent(loop);
	ez.settings.menuObj.addItem(_name, menu);
}


void ScreenShotFeature::menu() {
	bool updated = false;
	ezMenu menu(_name);
	menu.txtSmall();
	menu.buttons("up # back # select ## down #");
	menu.addItem(String("Select Trigger\t") + _triggers[_trigger]);
	menu.addItem(String("Jpg or Png?\t\t") + _extensions[_extension]);
	menu.addItem(String("Save to SPIFFS or SD?\t") + _fileSystems[_fs]);
	menu.addItem(String("File Name\t") + _fileName);
	while(menu.runOnce()) {
		switch(menu.pick()) {
			case 1: {
				String str("Select Trigger\t");
				if(++_trigger >= sizeof(_triggers) / sizeof(String)) _trigger = 0;
				menu.setCaption(1, String("Select Trigger\t") + _triggers[_trigger]);
				updated = true;
			}
			break;
			case 2: {
				_extension = _extension ? 0 : 1;    // good enough as long as there are only two
				updated = true;
				menu.setCaption(2, String("Jpg or Png?\t\t") + _extensions[_extension]);
			}
			break;
			case 3: {
				_fs = _fs ? 0 : 1;    // good enough as long as there are only two
				updated = true;
				menu.setCaption(3, String("Save to SPIFFS or SD?\t") + _fileSystems[_fs]);
			}
			break;
			case 4: {
				_fileName = ez.textInput("ScreenShot File Name", _fileName);
				updated = true;
				menu.setCaption(5, String("File Name\t") + _fileName);
			}
		}
	}
	if(updated) _writePrefs();
}


uint16_t ScreenShotFeature::loop() {
	return 1000;
}


void ScreenShotFeature::_readPrefs() {
	Preferences prefs;
	prefs.begin("M5ez", true);	// read-only
	_trigger = prefs.getChar("x_ss_trigger", 0);
	_extension = prefs.getChar("x_ss_extension", 0);
	_fs = prefs.getChar("x_ss_fs", 1);
	_fileName = prefs.getString("x_ss_filename", "/ss/ss_%00n");
	prefs.end();
}


void ScreenShotFeature::_writePrefs() {
	Preferences prefs;
	prefs.begin("M5ez", false);	// read-write
	prefs.putChar("x_ss_trigger", _trigger);
	prefs.putChar("x_ss_extension", _extension);
	prefs.getChar("x_ss_fs", _fs);
	prefs.putString("x_ss_filename", _fileName);
	prefs.end();
}
