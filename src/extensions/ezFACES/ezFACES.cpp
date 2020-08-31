#include <Preferences.h>
#include "../../M5ez.h"
#include "ezFACES.h"


bool ezFACES::_on;

bool ezFACES::control(uint8_t command, void* user) {
    switch(command) {
        case EXTENSION_CONTROL_PING:
			return true;
        case EXTENSION_CONTROL_START:
            begin();
            return true;
		case EXTENSION_CONTROL_QUERY_ENABLED:
			return on();
		case EXTENSION_CONTROL_FACES_POLL:
			if(nullptr == user) {
				poll();
				return true;
			}
			else {
				String* p_str = (String*)user;
				*p_str = poll();
				return 0 < p_str->length();
			}
    }
    return false;
}

void ezFACES::begin() {
	Preferences prefs;
	prefs.begin("M5ez", true);	// read-only
	_on = prefs.getBool("faces_on", false);
	prefs.end();
	if (_on) {
		Wire.begin();
		pinMode(5, INPUT);
		digitalWrite(5,HIGH);
		Wire.flush();
	}
	ez.settings.menuObj.addItem("FACES keyboard", ezFACES::menu);
}

void ezFACES::menu() {
	bool start_state = _on;
	while (true) {
		ezMenu facesmenu ("FACES keyboard");
		facesmenu.txtSmall();
		facesmenu.buttons("up#Back#select##down#");
		facesmenu.addItem("on|FACES keyboard\t" + (String)(_on ? "attached" : "not attached"));
		switch (facesmenu.runOnce()) {
			case 1:
				_on = !_on;
				if (_on) {
					pinMode(5, INPUT);
					digitalWrite(5,HIGH);
					Wire.flush();
				}
				break;
			case 0:
				if (_on != start_state) {
					Preferences prefs;
					prefs.begin("M5ez");
					prefs.putBool("faces_on", _on);
					prefs.end();
				}
				return;
			//
		}
	}
}

String ezFACES::poll() {
	if (digitalRead(5) == LOW) {
		Wire.requestFrom(0x88, 1);
		String out = "";
		while (Wire.available()) {
			out += (char) Wire.read();
		}
		ez.extensionControl("ezBacklight", EXTENSION_CONTROL_PING, nullptr);
		return out;
	}
	return "";
}

bool ezFACES::on() {
	return _on;
}
