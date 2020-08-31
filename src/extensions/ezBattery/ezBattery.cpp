#include <Preferences.h>
#include "../../M5ez.h"
#include "ezBattery.h"


bool ezBattery::_on = false;

bool ezBattery::control(uint8_t command, void* /* user */) {
    switch(command) {
        case EXTENSION_CONTROL_PING:
            return true;
        case EXTENSION_CONTROL_START:
            begin();
            return true;
        case EXTENSION_CONTROL_STOP:
            if(_on) {
                _on = false;
                _refresh();
            }
            return true;
        case EXTENSION_CONTROL_QUERY_ENABLED:
            return _on;
    }
    return false;
}

void ezBattery::begin() {
    Wire.begin();
    ezBattery::_readFlash();
    ez.settings.menuObj.addItem("Battery settings", ezBattery::menu);
    if (_on) {
        _refresh();
    }
}

void ezBattery::menu() {
    bool on_orig = _on;
    while(true) {
        ezMenu clockmenu("Battery Settings");
        clockmenu.txtSmall();
        clockmenu.buttons("up#Back#select##down#");
        clockmenu.addItem("on|Display battery\t" + (String)(_on ? "on" : "off"));
        switch (clockmenu.runOnce()) {
            case 1:
                _on = !_on;
                _refresh();
                break;
            case 0:
                if (_on != on_orig) {
                    _writeFlash();
                }
                return;
        }
    }
}

uint16_t ezBattery::loop() {
    if (!_on) return 0;
    ez.header.draw("battery");
    return 5000;
}

void ezBattery::_readFlash() {
    Preferences prefs;
    prefs.begin("M5ez", true);	// true: read-only
    _on = prefs.getBool("battery_icon_on", false);
    prefs.end();
}

void ezBattery::_writeFlash() {
    Preferences prefs;
    prefs.begin("M5ez");
    prefs.putBool("battery_icon_on", _on);
    prefs.end();
}

//Transform the M5Stack built in battery level into an internal format.
// From [100, 75, 50, 25, 0] to [4, 3, 2, 1, 0]
uint8_t ezBattery::_getTransformedBatteryLevel() {
    switch (m5.Power.getBatteryLevel()) {
        case 100:
            return 4;
        case 75:
            return 3;
        case 50:
            return 2;
        case 25:
            return 1;
        default:
            return 0;
    }
}

//Return the theme based battery bar color according to its level
uint32_t ezBattery::_getBatteryBarColor(uint8_t batteryLevel) {
    switch (batteryLevel) {
        case 0:
            return ez.theme->battery_0_fgcolor;
        case 1:
            return ez.theme->battery_25_fgcolor;
        case 2:
            return ez.theme->battery_50_fgcolor;
        case 3:
            return ez.theme->battery_75_fgcolor;
        case 4:
            return ez.theme->battery_100_fgcolor;
        default:
            return ez.theme->header_fgcolor;
    }
}

void ezBattery::_refresh() {
    if (_on) {
        ez.header.insert(RIGHTMOST, "battery", ez.theme->battery_bar_width + 2 * ez.theme->header_hmargin, ezBattery::_drawWidget);
        ez.addEvent(ezBattery::loop);
    } else {
        ez.header.remove("battery");
        ez.removeEvent(ezBattery::loop);
    }
}

void ezBattery::_drawWidget(uint16_t x, uint16_t w) {
    uint8_t currentBatteryLevel = _getTransformedBatteryLevel();
    uint16_t left_offset = x + ez.theme->header_hmargin;
    uint8_t top = ez.theme->header_height / 10;
    uint8_t height = ez.theme->header_height * 0.8;
    m5.lcd.fillRoundRect(left_offset, top, ez.theme->battery_bar_width, height, ez.theme->battery_bar_gap, ez.theme->header_bgcolor);
    m5.lcd.drawRoundRect(left_offset, top, ez.theme->battery_bar_width, height, ez.theme->battery_bar_gap, ez.theme->header_fgcolor);
    uint8_t bar_width = (ez.theme->battery_bar_width - ez.theme->battery_bar_gap * 5) / 4.0;
    uint8_t bar_height = height - ez.theme->battery_bar_gap * 2;
    left_offset += ez.theme->battery_bar_gap;
    for (uint8_t n = 0; n < currentBatteryLevel; n++) {
        m5.lcd.fillRect(left_offset + n * (bar_width + ez.theme->battery_bar_gap), top + ez.theme->battery_bar_gap,
            bar_width, bar_height, _getBatteryBarColor(currentBatteryLevel));
    }
}
