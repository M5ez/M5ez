#include <Preferences.h>
#include "../../M5ez.h"
#include "ezClock.h"


Timezone ezClock::tz;
bool ezClock::_on;
String ezClock::_timezone;
bool ezClock::_clock12;
bool ezClock::_am_pm;
String ezClock::_datetime;
bool ezClock::_starting = true;

void ezClock::begin() {
	Preferences prefs;
	prefs.begin("M5ez", true);	// read-only
	_on = prefs.getBool("clock_on", true);
	_timezone = prefs.getString("timezone", "GeoIP");
	_clock12 = prefs.getBool("clock12", false);
	_am_pm = prefs.getBool("ampm", false);
	prefs.end();
	ez.settings.menuObj.addItem("Clock settings", ezClock::menu);
	ez.addEvent(ezClock::loop);
	restart();
}

void ezClock::restart() {
	ez.header.remove("clock");
	uint8_t length;
	if (_on) {
		if (_clock12) {
			if (_am_pm) {
				_datetime = "g:ia";
				length = 7;
			} else {
				_datetime = "g:i";
				length = 5;
			}
		} else {
			_datetime = "H:i";
			length = 5;
		}
		ez.setFont(ez.theme->clock_font);
		uint8_t width = length * m5.lcd.textWidth("5") + ez.theme->header_hmargin * 2;
		ez.header.insert(RIGHTMOST, "clock", width, ezClock::draw);
	}
}

void ezClock::menu() {
	bool on_orig = _on;
	bool clock12_orig = _clock12;
	bool am_pm_orig = _am_pm;
	String tz_orig = _timezone;
	while(true) {
		ezMenu clockmenu("Clock Settings");
		clockmenu.txtSmall();
		clockmenu.buttons("up#Back#select##down#");
		clockmenu.addItem("on|Display clock\t" + (String)(_on ? "on" : "off"));
		if (_on) {
			clockmenu.addItem("tz|Timezone\t" + _timezone);
			clockmenu.addItem("1224|12/24 hour\t" + (String)(_clock12 ? "12" : "24"));
			if (_clock12) {
				clockmenu.addItem("ampm|am/pm indicator\t" + (String)(_am_pm ? "on" : "off"));
			}
		}
		switch (clockmenu.runOnce()) {
			case 1:
				_on = !_on;
				ezClock::restart();
				break;
			case 2:
				_timezone = ez.textInput("Enter timezone");
				if (_timezone == "") _timezone = "GeoIP";
				if (tz.setLocation(_timezone)) _timezone = tz.getOlsen();
				break;
			case 3:
				_clock12 = !_clock12;
				ezClock::restart();
				break;
			case 4:
				_am_pm = !_am_pm;
				ezClock::restart();
				break;
			case 0:
				if (_am_pm != am_pm_orig || _clock12 != clock12_orig || _on != on_orig || _timezone != tz_orig) {
					_writePrefs();
				}
				return;
		}
	}
}

uint16_t ezClock::loop() {
	ezt::events();
	if (_starting && timeStatus() != timeNotSet) {
		_starting = false;
		if (tz.setLocation(_timezone)) {
			if (tz.getOlsen() != _timezone) {
				_timezone = tz.getOlsen();
				_writePrefs();
			}
		}
		ez.header.draw("clock");
	} else {
		if (_on && ezt::minuteChanged()) ez.header.draw("clock");
	}
	return 250;
}

void ezClock::draw(uint16_t x, uint16_t w) {
	if (_starting) return;
	m5.lcd.fillRect(x, 0, w, ez.theme->header_height, ez.theme->header_bgcolor);
	ez.setFont(ez.theme->clock_font);
	m5.lcd.setTextColor(ez.theme->header_fgcolor);
	m5.lcd.setTextDatum(TL_DATUM);
	m5.lcd.drawString(tz.dateTime(_datetime), x + ez.theme->header_hmargin, ez.theme->header_tmargin + 2);
}

void ezClock::_writePrefs() {
	Preferences prefs;
	prefs.begin("M5ez");
	prefs.putBool("clock_on", _on);
	prefs.putString("timezone", _timezone);
	prefs.putBool("clock12", _clock12);
	prefs.putBool("ampm", _am_pm);
	prefs.end();
}

bool ezClock::waitForSync(const uint16_t timeout /* = 0 */) {

	unsigned long start = millis();
	ez.msgBox("Clock sync", "Waiting for clock synchronisation", "", false);	
	while (ezt::timeStatus() != timeSet) {
		if ( timeout && (millis() - start) / 1000 > timeout ) return false;
		delay(25);
		ez.yield();
	}
	return true;
}
