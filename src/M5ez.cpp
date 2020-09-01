#include <M5ez.h>

#include <Preferences.h>

#include "extensions/ezWifi/ezWifi.h"
#include "extensions/ezFACES/ezFACES.h"
#include "extensions/ezBacklight/ezBacklight.h"
#include "extensions/ezClock/ezClock.h"
#include "extensions/ezBattery/ezBattery.h"
#include "extensions/ezBLE/ezBLE.h"


#include <algorithm>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   T H E M E
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ezTheme::begin() {
	if (!ez.themes.size()) {
		ezTheme defaultTheme;
		defaultTheme.add();
	}
	ez.theme = &ez.themes[0];
	Preferences prefs;
	prefs.begin("M5ez", true);	// read-only
	select(prefs.getString("theme", ""));
	prefs.end();
}

void ezTheme::add() {
	ez.themes.push_back(*this);
}

bool ezTheme::select(String name) {
	for (uint8_t n = 0; n < ez.themes.size(); n++) {
		if (ez.themes[n].name == name) {
			ez.theme = &ez.themes[n];
			return true;
		}
	}
	return false;
}

void ezTheme::menu() {
	String orig_name = ez.theme->name;
	ezMenu thememenu("Theme chooser");
	thememenu.txtSmall();
	thememenu.buttons("up#Back#select##down#");
	for (uint8_t n = 0; n < ez.themes.size(); n++) {
		thememenu.addItem(ez.themes[n].name);
	}
	while(thememenu.runOnce()) {
		if (thememenu.pick()) {
			ez.theme->select(thememenu.pickName());
		}
	}
	if (ez.theme->name != orig_name) {
		Preferences prefs;
		prefs.begin("M5ez");
		prefs.putString("theme", ez.theme->name);
		prefs.end();
	}
	return;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   S C R E E N
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t ezScreen::_background;

void ezScreen::begin() {
	_background = ez.theme->background;
	ez.header.begin();
	ez.canvas.begin();
	ez.buttons.begin();
}

uint16_t ezScreen::background() { return _background; }

void ezScreen::clear() { clear(ez.theme->background); }

void ezScreen::clear(uint16_t color) {
	_background = color;
	ez.header.clear(false);
	ez.buttons.clear(false);
	ez.canvas.reset();
	m5.lcd.fillRect(0, 0, TFT_W, TFT_H, color);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   H E A D E R
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<header_widget_t> ezHeader::_widgets;
bool ezHeader::_shown;
String ezHeader::_title;

void ezHeader::begin() {
	_widgets.clear();
	insert(0, "title", 0, _drawTitle, true);
	_shown = false;
}

void ezHeader::_recalculate() {
	bool we_have_leftover = false;
	uint16_t x = 0;
	for (uint8_t n = 0; n < _widgets.size(); n++) {				// start from left, set x values
		_widgets[n].x = x;
		if (_widgets[n].leftover) {								// until "leftover" widget (usually "title")
			we_have_leftover = true;
			break;
		}
		x += _widgets[n].w;
	}
	if (we_have_leftover) {										// Then start from right setting x values
		x = TFT_W;
		for (int8_t n = _widgets.size() - 1; n >= 0 ; n--) {
			if (_widgets[n].leftover) {							// and set width of leftover widget to remainder
				_widgets[n].w = x - _widgets[n].x;
				break;
			}
			x -= _widgets[n].w;
			_widgets[n].x = x;
		}
	}
	if (_shown) show();
}

void ezHeader::insert(uint8_t position, String name, uint16_t width, void (*function)(uint16_t x, uint16_t w), bool leftover /* = false */) {
	for (uint8_t n = 0; n < _widgets.size(); n++) {
		if (_widgets[n].leftover) leftover = false;			// ignore leftover if there already is one
		if (_widgets[n].name == name) return;				// fail silently if trying to create two widgets with same name
	}
	if (position > _widgets.size()) position = _widgets.size();		// interpret anything over current number of widgets as RIGHTMOST
	header_widget_t wdgt;
	wdgt.name = name;
	wdgt.leftover = leftover;
	wdgt.x = 0;
	wdgt.w = width;
	wdgt.function = function;
	_widgets.insert(_widgets.begin() + position, wdgt);
	_recalculate();
}

void ezHeader::remove(String name) {
	for (uint8_t n = 0; n < _widgets.size(); n++) {
		if (_widgets[n].name == name) {
			_widgets.erase(_widgets.begin() + n);
			_recalculate();
		}
	}
}

uint8_t ezHeader::position(String name) {
	for (uint8_t n = 0; n < _widgets.size(); n++) {
		if (_widgets[n].name == name) return n;
	}
	return 0;
}

void ezHeader::show(String t /* = "" */) {
	_shown = true;
	if (t != "") _title = t;											// only change title if provided
	m5.lcd.fillRect(0, 0, TFT_W, ez.theme->header_height, ez.theme->header_bgcolor);	// Clear header area
	for (uint8_t n = 0; n < _widgets.size(); n++) {
		(_widgets[n].function)(_widgets[n].x, _widgets[n].w);		// Tell all header widgets to draw
	}
	ez.canvas.top(ez.theme->header_height);
}

void ezHeader::draw(String name) {
	if (!_shown) return;
	for (uint8_t n = 0; n < _widgets.size(); n++) {
		if (_widgets[n].name == name) {
			(_widgets[n].function)(_widgets[n].x, _widgets[n].w);
		}
	}
}

void ezHeader::clear(bool wipe /* = true */) {
	if (wipe) m5.lcd.fillRect(0, 0, TFT_W, ez.theme->header_height, ez.theme->background);
	_shown = false;
	ez.canvas.top(0);
}

bool ezHeader::shown() { return _shown; }

String ezHeader::title() {
	return _title;
}

void ezHeader::title(String t) {
	_title = t;
	if (_shown) draw("title");
}

void ezHeader::_drawTitle(uint16_t x, uint16_t w) {
	m5.lcd.fillRect(x, 0, w, ez.theme->header_height, ez.theme->header_bgcolor);
	m5.lcd.setTextDatum(TL_DATUM);
	m5.lcd.setTextColor(ez.theme->header_fgcolor);
	ez.setFont(ez.theme->header_font);
	m5.lcd.drawString(ez.clipString(_title, w - ez.theme->header_hmargin), x + ez.theme->header_hmargin, ez.theme->header_tmargin);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   C A N V A S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t ezCanvas::_y, ezCanvas::_top, ezCanvas::_bottom;
uint16_t ezCanvas::_x, ezCanvas::_left, ezCanvas::_right, ezCanvas::_lmargin;
const GFXfont* ezCanvas::_font;
uint16_t ezCanvas::_color;
bool ezCanvas::_wrap, ezCanvas::_scroll;
std::vector<print_t> ezCanvas::_printed;
uint32_t ezCanvas::_next_scroll;

void ezCanvas::begin() {
	_left = 0;
	_right = TFT_W - 1;
	_top = 0;
	_bottom = TFT_H - 1;
	ez.addEvent(ez.canvas.loop);
	reset();
}

void ezCanvas::reset() {
	_wrap = true;
	_scroll = false;
	_font = ez.theme->print_font;
	_color = ez.theme->print_color;
	_lmargin = 0;
	_next_scroll = 0;
	clear();
}

void ezCanvas::clear() {
	m5.lcd.fillRect(left(), top(), width(), height(), ez.screen.background());
	_x = _lmargin;
	_y = 0;
	_printed.clear();
}

uint8_t ezCanvas::top() { return _top; }

uint8_t ezCanvas::bottom() { return _bottom; }

uint16_t ezCanvas::left() { return _left; }

uint16_t ezCanvas::right() { return _right; }

uint8_t ezCanvas::height() { return _bottom - _top + 1;}

uint16_t ezCanvas::width() { return _right - _left + 1; }

bool ezCanvas::scroll() { return _scroll; }

void ezCanvas::scroll(bool s) { _scroll = s; }

bool ezCanvas::wrap() { return _wrap; }

void ezCanvas::wrap(bool w) { _wrap = w; }

uint16_t ezCanvas::lmargin() { return _lmargin; }

void ezCanvas::lmargin(uint16_t newmargin) {
	if (_x < newmargin) _x = newmargin;
	_lmargin = newmargin;
}

void ezCanvas::font(const GFXfont* font) { _font = font; }

const GFXfont* ezCanvas::font() { return _font; }

void ezCanvas::color(uint16_t color) { _color = color; }

uint16_t ezCanvas::color() { return _color; }

void ezCanvas::pos(uint16_t x, uint8_t y) {
	_x = x;
	_y = y;
}

uint16_t ezCanvas::x() { return _x; }

void ezCanvas::x(uint16_t x) { _x = x; }

uint8_t ezCanvas::y() { return _y; }

void ezCanvas::y(uint8_t y) { _y = y; }

void ezCanvas::top(uint8_t newtop) {
	if (_y < newtop) _y = newtop;
	_top = newtop;
}

void ezCanvas::bottom(uint8_t newbottom) {
	_bottom = newbottom;
}

size_t ezCanvas::write(uint8_t c) {
	String tmp = String((char)c);
	_print(tmp);
	return 1;
}

size_t ezCanvas::write(const char *str) {
	String tmp = String(str);
	_print(tmp);
	return sizeof(str);
}

size_t ezCanvas::write(const uint8_t *buffer, size_t size) {
	String tmp;
	tmp.reserve(size);
	for (uint16_t n = 0; n < size ; n++) tmp += (char)*(buffer + n);
	_print(tmp);
	return size;
}

uint16_t ezCanvas::loop() {
	if (_next_scroll && millis() >= _next_scroll) {
		ez.setFont(_font);
		uint8_t h = ez.fontHeight();
		uint8_t scroll_by = _y - _bottom;
		if (_x > _lmargin) scroll_by += h;
		const GFXfont* hold_font = _font;
		const uint16_t hold_color = _color;
		for (uint16_t n = 0; n < _printed.size(); n++) {
			_printed[n].y -= scroll_by;
		}
		m5.lcd.fillRect(left(), top(), width(), height(), ez.screen.background());
		// m5.lcd.fillRect(0, 0, 320, 240, ez.screen.background());
		for (uint16_t n = 0; n < _printed.size(); n++) {
			if (_printed[n].y >= _top) {
				if (_printed[n].font != _font) ez.setFont(_printed[n].font);
				if (_printed[n].color != _color) m5.lcd.setTextColor(_printed[n].color);
				m5.lcd.drawString(_printed[n].text, _printed[n].x, _printed[n].y);
			}
		}
		_y -= scroll_by;
		_font = hold_font;
		_color = hold_color;
		_next_scroll = 0;

		std::vector<print_t> clean_copy;
		for (uint16_t n = 0; n < _printed.size(); n++) {
			if (_printed[n].y >= 0) clean_copy.push_back(_printed[n]);
		}
		_printed = clean_copy;
		Serial.println(ESP.getFreeHeap());
	}
	return 10;
}


void ezCanvas::_print(String text) {
 	ez.setFont(_font);
 	m5.lcd.setTextDatum(TL_DATUM);
	m5.lcd.setTextColor(_color, ez.theme->background);
 	uint8_t h = ez.fontHeight();
 	if (_y + h > _bottom) {
 		if (!_scroll) return;
 		if (!_next_scroll) _next_scroll = millis() + 200;
 	}
 	int16_t crlf = text.indexOf("\r\n");
 	String remainder = "";
 	if (crlf != -1) {
 		remainder = text.substring(crlf + 2);
 		text = text.substring(0,crlf);
 	}
 	if (_x + m5.lcd.textWidth(text) <= _right) {
		if (text != "") _putString(text);
	} else {
		for (uint16_t n = 0; n < text.length(); n++) {
			if (_x + m5.lcd.textWidth(text.substring(0, n + 1)) > _right) {
				if (n) {
					_putString(text.substring(0, n));
				}
				if (_wrap) {
					_x = _lmargin;
					_y += h;
					_print(text.substring(n));
				}
				break;
			}
		}
	}
	if (crlf != -1) {
		_x = _lmargin;
		_y += h;
		if (remainder != "") _print (remainder);
	}
}

void ezCanvas::_putString(String text) {
	ez.setFont(_font);
	uint8_t h = ez.fontHeight();
	if (_scroll) {
		print_t p;
		p.font = _font;
		p.color = _color;
		p.x = _x;
		p.y = _y;
		p.text = text;
		_printed.push_back(p);
	}
	if (_y + h > _bottom) {
		_x += m5.lcd.textWidth(text);
	} else{
		_x += m5.lcd.drawString(text, _x, _y);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   B U T T O N S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String ezButtons::_btn_a_s, ezButtons::_btn_a_l;
String ezButtons::_btn_b_s, ezButtons::_btn_b_l;
String ezButtons::_btn_c_s, ezButtons::_btn_c_l;
String ezButtons::_btn_ab, ezButtons::_btn_bc, ezButtons::_btn_ac;
bool ezButtons::_key_release_wait;
bool ezButtons::_lower_button_row, ezButtons::_upper_button_row;

void ezButtons::begin() { clear(false); }

void ezButtons::show(String buttons) {
	buttons.trim();
	std::vector<String> buttonVector;
	ez.chopString(buttons, "#", buttonVector, true);
	switch (buttonVector.size()) {
		case 1:
			_drawButtons("", "", buttons, "", "", "", "", "", "");
			break;
		case 3:
			// Three elements, so shortpress only
			_drawButtons(buttonVector[0], "", buttonVector[1], "", buttonVector[2], "", "", "", "");
			break;
		case 6:
			// Six elements, so all buttons long and short
			_drawButtons(buttonVector[0], buttonVector[1], buttonVector[2], buttonVector[3], buttonVector[4], buttonVector[5], "", "", "");
			break;
		case 9:
			// Nine elements, so all buttons long and short plus the top row of three multi-keys
			_drawButtons(buttonVector[0], buttonVector[1], buttonVector[2], buttonVector[3], buttonVector[4], buttonVector[5], buttonVector[6], buttonVector[7], buttonVector[8]);
			break;
	}
}

void ezButtons::clear(bool wipe /* = true */) {
	if (wipe && (_lower_button_row  || _upper_button_row)) {
		m5.lcd.fillRect(0, ez.canvas.bottom() + 1, TFT_H - ez.canvas.bottom() - 1, TFT_W, ez.screen.background());
	}
	_btn_a_s = _btn_a_l = _btn_b_s = _btn_b_l = _btn_c_s = _btn_c_l = "";
	_btn_ab = _btn_bc = _btn_ac = "";
	_lower_button_row = false;
	_upper_button_row = false;
	ez.canvas.bottom(TFT_H - 1);
}

void ezButtons::_drawButtons(String btn_a_s, String btn_a_l, String btn_b_s, String btn_b_l, String btn_c_s, String btn_c_l, String btn_ab, String btn_bc, String btn_ac) {
	int16_t btnwidth = int16_t( (TFT_W - 4 * ez.theme->button_gap ) / 3);

	// See if any buttons are used on the bottom row
	if (btn_a_s != "" || btn_a_l != "" || btn_b_s != "" || btn_b_l != "" || btn_c_s != "" || btn_c_l != "") {
		if (!_lower_button_row) {
			// If the lower button row wasn't there before, clear the area first
			m5.lcd.fillRect(0, TFT_H - ez.theme->button_height - ez.theme->button_gap, TFT_W, ez.theme->button_height + ez.theme->button_gap, ez.screen.background());
		}
		// Then draw the three buttons there. (drawButton erases single buttons if unused.)
		if (_btn_a_s != btn_a_s || _btn_a_l != btn_a_l) {
			_drawButton(1, ez.rightOf(btn_a_s, "|", true), ez.rightOf(btn_a_l, "|", true), ez.theme->button_gap, btnwidth);
			_btn_a_s = btn_a_s;
			_btn_a_l = btn_a_l;
		}
		if (_btn_b_s != btn_b_s || _btn_b_l != btn_b_l) {
			_drawButton(1, ez.rightOf(btn_b_s, "|", true), ez.rightOf(btn_b_l, "|", true), btnwidth + 2 * ez.theme->button_gap, btnwidth);
			_btn_b_s = btn_b_s;
			_btn_b_l = btn_b_l;
		}
		if (_btn_c_s != btn_c_s || _btn_c_l != btn_c_l) {
			_drawButton(1, ez.rightOf(btn_c_s, "|", true), ez.rightOf(btn_c_l, "|", true), 2 * btnwidth + 3 * ez.theme->button_gap, btnwidth);
			_btn_c_s = btn_c_s;
			_btn_c_l = btn_c_l;
		}
		_lower_button_row = true;
	} else {
		if (_lower_button_row) {
			// If there was a lower button row before and it's now gone, clear the area
			m5.lcd.fillRect(0, TFT_H - ez.theme->button_height - ez.theme->button_gap, TFT_W, ez.theme->button_height + ez.theme->button_gap, ez.screen.background());
			_btn_a_s = _btn_a_l = _btn_b_s = _btn_b_l = _btn_c_s = _btn_c_l = "";
			_lower_button_row = false;
		}
	}
	//Now check if there are multi-buttons used (top row)
	if (btn_ab != "" || btn_bc != "" || btn_ac != "") {
		if (!_upper_button_row) {
			// If the upper button row wasn't there before, clear the area first
			m5.lcd.fillRect(0, TFT_H - 2 * (ez.theme->button_height + ez.theme->button_gap), TFT_W, ez.theme->button_height + ez.theme->button_gap, ez.screen.background());
		}
		// Then draw the buttons
		if (_btn_ab != btn_ab) {
			_drawButton(2, ez.rightOf(btn_ab, "|", true), "", ez.theme->button_gap + (btnwidth / 2), btnwidth);
			_btn_ab = btn_ab;
		}
		if (_btn_bc != btn_bc) {
			_drawButton(2, ez.rightOf(btn_bc, "|", true), "", (2 * ez.theme->button_gap) + btnwidth + (btnwidth / 2), btnwidth);
			_btn_bc = btn_bc;
		}
		if (_btn_ac != btn_ac) {
			// Two halves of the same button

			// ugly in code, prettier on the screen: making the buttons square on the screen edges to signal wrap-around
			m5.lcd.fillRect(0, TFT_H - 2 * ez.theme->button_height - ez.theme->button_gap, btnwidth / 4, ez.theme->button_height, ez.theme->button_bgcolor_t);
			m5.lcd.fillRect(TFT_W - (btnwidth / 4), TFT_H - 2 * ez.theme->button_height - ez.theme->button_gap, btnwidth / 4, ez.theme->button_height, ez.theme->button_bgcolor_t);

			_drawButton(2, ez.rightOf(btn_ac, "|", true), "", (3 * ez.theme->button_gap) + (2 * btnwidth) + (btnwidth / 2), (btnwidth / 2));
			_drawButton(2, ez.rightOf(btn_ac, "|", true), "", 0, (btnwidth / 2));

			_btn_ac = btn_ac;
		}
		_upper_button_row = true;
	} else {
		if (_upper_button_row) {
			// If there was an upper button row before and it's now gone, clear the area
			m5.lcd.fillRect(0, TFT_H - 2 * (ez.theme->button_height + ez.theme->button_gap), TFT_W, ez.theme->button_height + ez.theme->button_gap, ez.screen.background());
			_btn_ab = _btn_bc = _btn_ac = "";
			_upper_button_row = false;
		}
	}

	uint8_t button_rows = _upper_button_row ? 2 : (_lower_button_row ? 1 : 0);
	ez.canvas.bottom(TFT_H - (button_rows * (ez.theme->button_height + ez.theme->button_gap)));
}

void ezButtons::_drawButton(int16_t row, String text_s, String text_l, int16_t x, int16_t w) {
	// row = 1 for lower and 2 for upper row
	int16_t y, bg_color;
	if (row == 1) {
		y = TFT_H - ez.theme->button_height;
		bg_color = ez.theme->button_bgcolor_b;
	} else {
		y = TFT_H - 2 * ez.theme->button_height - ez.theme->button_gap;
		bg_color = ez.theme->button_bgcolor_t;
	}
	if (text_s != "" || text_l != "") {
		ez.setFont(ez.theme->button_font);
		m5.lcd.fillRoundRect(x, y, w, ez.theme->button_height, ez.theme->button_radius, bg_color);
		if (text_l != "") {
			_drawButtonString(text_s, x + ez.theme->button_hmargin, y + ez.theme->button_tmargin, ez.theme->button_fgcolor, TL_DATUM);
		} else {
			_drawButtonString(text_s, x + int16_t(w / 2), y + ez.theme->button_tmargin, ez.theme->button_fgcolor, TC_DATUM);
		}
		_drawButtonString(text_l, x + w - ez.theme->button_hmargin, y + ez.theme->button_tmargin, ez.theme->button_longcolor, TR_DATUM);
	} else {
		m5.lcd.fillRect(x, y, w, ez.theme->button_height, ez.screen.background());
	}
}

void ezButtons::_drawButtonString(String text, int16_t x, int16_t y, uint16_t color, int16_t datum) {
	if (text == "~") return;
	if (text == "up" || text == "down" || text == "left" || text == "right") {
		y+=2;
		int16_t w = m5.lcd.textWidth("A") * 1.2;
		int16_t h = ez.fontHeight() * 0.6;
		if (datum == TR_DATUM) x = x - w;
		if (datum == TC_DATUM) x = x - w/2;
		if (text == "up") m5.lcd.fillTriangle(x, y + h, x + w, y + h, x + w/2 ,y , color);
		if (text == "down") m5.lcd.fillTriangle(x, y, x + w, y, x + w/2 ,y + h, color);
		if (text == "right") m5.lcd.fillTriangle(x, y, x, y + h, x + w, y + h/2, color);
		if (text == "left") m5.lcd.fillTriangle(x + w, y, x + w, y + h, x, y + h/2, color);
	} else {
		m5.lcd.setTextColor(color);
		m5.lcd.setTextDatum(datum);
		m5.lcd.drawString(text, x, y);
	}
}

void ezButtons::releaseWait() {
	_key_release_wait = true;
}

String ezButtons::poll() {
	String keystr = "";

	ez.yield();

	if (!_key_release_wait) {
		if (_btn_ab != "" && m5.BtnA.isPressed() && m5.BtnB.isPressed() ) {
			keystr = ez.leftOf(_btn_ab, "|", true);
			_key_release_wait = true;
		}
		if (_btn_bc != "" && m5.BtnB.isPressed() && m5.BtnC.isPressed() ) {
			keystr = ez.leftOf(_btn_bc, "|", true);
			_key_release_wait = true;
		}
		if (_btn_ac != "" && m5.BtnA.isPressed() && m5.BtnC.isPressed() ) {
			keystr = ez.leftOf(_btn_ac, "|", true);
			_key_release_wait = true;
		}

		if (_btn_a_l != "" && m5.BtnA.pressedFor(ez.theme->longpress_time) ) {
			keystr = ez.leftOf(_btn_a_l, "|", true);
			_key_release_wait = true;
		}
		if (_btn_a_s != "" && m5.BtnA.wasReleased() ) {
			keystr = ez.leftOf(_btn_a_s, "|", true);
		}

		if (_btn_b_l != "" && m5.BtnB.pressedFor(ez.theme->longpress_time) ) {
			keystr = ez.leftOf(_btn_b_l, "|", true);
			_key_release_wait = true;
		}
		if (_btn_b_s != "" && m5.BtnB.wasReleased() ) {
			keystr = ez.leftOf(_btn_b_s, "|", true);
		}

		if (_btn_c_l != "" && m5.BtnC.pressedFor(ez.theme->longpress_time) ) {
			keystr = ez.leftOf(_btn_c_l, "|", true);
			_key_release_wait = true;
		}
		if (_btn_c_s != "" && m5.BtnC.wasReleased() ) {
			keystr = ez.leftOf(_btn_c_s, "|", true);
		}
	}

	if (m5.BtnA.isReleased() && m5.BtnB.isReleased() && m5.BtnC.isReleased() ) {
		_key_release_wait = false;
	}

	if (keystr == "~") keystr = "";
	if (keystr != "") ez.extensionControl("ezBacklight", EXTENSION_CONTROL_PING, nullptr);
	return keystr;
}

String ezButtons::wait(String buttons /* = "" */) {
	if (buttons != "") show(buttons);
	String keystr = "";
	while (keystr == "") {
		keystr = ez.buttons.poll();
	}
	return keystr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   S E T T I N G S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ezMenu ezSettings::menuObj ("Settings menu");

void ezSettings::begin() {
	menuObj.txtSmall();
	menuObj.buttons("up#Back#select##down#");
	if (ez.themes.size() > 1) {
		ez.settings.menuObj.addItem("Theme chooser", ez.theme->menu);
	}
	// Install all extensions
	for(auto& ext : M5ez::extensions) { 
    	ext.control(EXTENSION_CONTROL_START, nullptr);
	}
	ez.settings.menuObj.addItem("Factory defaults", ez.settings.defaults);
}

void ezSettings::menu() {
	menuObj.run();
}

void ezSettings::defaults() {
	if (ez.msgBox("Reset to defaults?", "Are you sure you want to reset all settings to factory defaults?", "yes##no") == "yes") {
		Preferences prefs;
		prefs.begin("M5ez");
		prefs.clear();
		prefs.end();
		ESP.restart();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   E Z
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ezTheme> M5ez::themes;
ezTheme* M5ez::theme = NULL;
ezScreen M5ez::screen;
constexpr ezScreen& M5ez::s;
ezHeader M5ez::header;
constexpr ezHeader& M5ez::h;
ezCanvas M5ez::canvas;
constexpr ezCanvas& M5ez::c;
ezButtons M5ez::buttons;
constexpr ezButtons& M5ez::b;
ezSettings M5ez::settings;
ezMenu* M5ez::_currentMenu = nullptr;
bool M5ez::_in_event = false;
std::vector<event_t> M5ez::_events;
std::vector<extension_t> M5ez::extensions = { {"ezWifi", ezWifi::control}, {"ezFACES", ezFACES::control},
											{"ezBacklight", ezBacklight::control}, {"ezClock", ezClock::control},
											{"ezBattery", ezBattery::control}, {"ezBLE", ezBLE::control} };

// ez.textInput
int16_t M5ez::_text_cursor_x, M5ez::_text_cursor_y, M5ez::_text_cursor_h, M5ez::_text_cursor_w;
bool M5ez::_text_cursor_state;
long  M5ez::_text_cursor_millis;

void M5ez::begin() {
	m5.begin();
	ezTheme::begin();
	ez.screen.begin();
	ez.settings.begin();
}

void M5ez::yield() {
	::yield();			// execute the Arduino yield in the root namespace
	M5.update();
	if(M5ez::_in_event) return;			// prevent reentrancy
	for (uint8_t n = 0; n< _events.size(); n++) {
		if (millis() > _events[n].when) {
			M5ez::_in_event = true;		// prevent reentrancy
			uint16_t r = (_events[n].function)();
			M5ez::_in_event = false;	// prevent reentrancy
			if (r) {
				_events[n].when = millis() + r - 1;
			} else {
				_events.erase(_events.begin() + n);
				break;		// make sure we don't go beyond _events.size() after deletion
			}
		}
	}
	events();		// This was #ifdef M5EZ_CLOCK, with a comment: TMP
}

void M5ez::addEvent(uint16_t (*function)(), uint32_t when /* = 1 */) {
	event_t n;
	n.function = function;
	n.when = millis() + when - 1;
	_events.push_back(n);
}

void M5ez::removeEvent(uint16_t (*function)()) {
	uint8_t n = 0;
	while (n < _events.size()) {
		if (_events[n].function == function) {
			_events.erase(_events.begin() + n);
		}
		n++;
	}
}

bool M5ez::_redraw;

void M5ez::redraw() { _redraw = true; }

ezMenu* M5ez::getCurrentMenu() { return _currentMenu; }


static const char * _keydefs[] PROGMEM = {
	"KB3|qrstu.#SP#KB4|vwxyz,#Del#KB5|More#LCK:|Lock#KB1|abcdefgh#KB2|ijklmnop#Done",	//KB0
	"c#d#e#f#g#h#a#b#Back",																//KB1
	"k#l#m#n#o#p#i#j#Back",																//KB2
	"s#t#u#.###q#r#Back",																//KB3
	"x#y#z#,###v#w#Back",																//KB4
	"KB8|Q-U.#SP#KB9|V-Z,#Del#KB10|More#LCK:CAPS|Lock#KB6|A-H#KB7|I-P#Done",			//KB5
	"C#D#E#F#G#H#A#B#Back",																//KB6
	"K#L#M#N#O#P#I#J#Back",																//KB7
	"S#T#U#.###Q#R#Back",																//KB8
	"X#Y#Z#,###V#W#Back",																//KB9
	"KB11|1-5.#SP#KB12|6-0,#Del#KB13|More#LCK:NUM|Lock###Done",							//KB10
	"1#2#3#4#5#,###Back",																//KB11
	"6#7#8#9#0#.###Back",																//KB12
	"KB14|!?:;\\#$^&#SP#KB15|*()_-+=\\|#Del#KB0|More#LCK:SYM|Lock#KB16|'\"`@%\\/#KB17|<>{}[]()#Done",	//KB13
	"!#?#:#;#\\##$#^#&#Back",															//KB14
	"*#(#)#_#-#+#=#\\|#Back",															//KB15
	"'#\"#`#@#%#/###Back",																//KB16
	"<#>#{#}#[#]#(#)#Back",																//KB17
	".#,#Del##Done#"																	//KB18
};

// ez.msgBox

String M5ez::msgBox(String header, String msg, String buttons /* = "OK" */, const bool blocking /* = true */, const GFXfont* font /* = NULL */, uint16_t color /* = NO_COLOR */) {
	if (ez.header.title() != header) {
		ez.screen.clear();
		if (header != "") ez.header.show(header);
	} else {
		ez.canvas.clear();
	}
	if (!font) font = ez.theme->msg_font;
	if (color == NO_COLOR) color = ez.theme->msg_color;
	ez.buttons.show(buttons);
	std::vector<line_t> lines;
	msg.replace("|", (String)char(13));
	m5.lcd.setTextDatum(CC_DATUM);
	m5.lcd.setTextColor(color);
	ez.setFont(font);
	_fitLines(msg, ez.canvas.width() - 2 * ez.theme->msg_hmargin, ez.canvas.width() / 3, lines);
	int16_t font_h = ez.fontHeight();
	for (int8_t n = 0; n < lines.size(); n++) {
		int16_t y = ez.canvas.top() + ez.canvas.height() / 2 - ( (lines.size() - 1) * font_h / 2) + n * font_h;
		m5.lcd.drawString(lines[n].line, TFT_W / 2, y);
	}
	if (buttons != "" && blocking) {
		String ret = ez.buttons.wait();
		ez.screen.clear();
		return ret;
	} else {
		return "";
	}
}


// ez.textInput

String M5ez::textInput(String header /* = "" */, String defaultText /* = "" */) {

	int16_t current_kb = 0, prev_kb = 0, locked_kb = 0;
	if(ez.extensionControl("ezFACES", EXTENSION_CONTROL_QUERY_ENABLED, nullptr)) {
		current_kb = locked_kb = prev_kb = ez.theme->input_faces_btns;
		ez.extensionControl("ezFACES", EXTENSION_CONTROL_FACES_POLL, nullptr);	// flush key buffer in FACES
	}
	String tmpstr;
	String text = defaultText;
	ez.screen.clear();
	if (header != "") ez.header.show(header);
	_drawTextInputBox(text);
	String key;
	ez.buttons.show(_keydefs[current_kb]);

	while (true) {
		key = ez.buttons.poll();
		if(ez.extensionControl("ezFACES", EXTENSION_CONTROL_QUERY_ENABLED, nullptr)) {
			ez.extensionControl("ezFACES", EXTENSION_CONTROL_FACES_POLL, (void*)&key);
		}
		if (key == "Done" || key == (String)char(13)) return text;
		if (key.substring(0, 2) == "KB") {
			prev_kb = current_kb;
			tmpstr = key.substring(2);
			current_kb = tmpstr.toInt();
			ez.buttons.show(_keydefs[current_kb]);
			key = "";
		}
		if (key.substring(0, 4) == "LCK:") {
			if (locked_kb != current_kb) {
				_drawTextInputLockString( key.substring(4) );
				locked_kb = current_kb;
			} else {
				_drawTextInputLockString( "" );
				locked_kb = 0;
				current_kb = 0;
				ez.buttons.show(_keydefs[current_kb]);
			}
			key = "";
		}
		if (key == "Back") {
			current_kb = prev_kb;
			ez.buttons.show(_keydefs[current_kb]);
			key = "";
		}
		if (key == "Del" || key == (String)char(8) || key == (String)char(127)) {
			text = text.substring(0, text.length() - 1);
			_drawTextInputBox(text);
			key = "";
		}
		if (key == "SP") key = " ";
		if (key >= " " && key <= "~") {
			current_kb = locked_kb;
			ez.buttons.show(_keydefs[current_kb]);
			text += key;
			_drawTextInputBox(text);
		}
		_textCursor();
	}
}

void M5ez::_drawTextInputLockString(String text) {
	m5.lcd.setTextColor(TFT_RED);
	ez.setFont(ez.theme->input_keylock_font);
	m5.lcd.setTextDatum(TR_DATUM);
	int16_t text_h = ez.fontHeight();
	m5.lcd.fillRect(0, _text_cursor_y + ez.theme->input_vmargin + 10 + text_h, TFT_W, text_h, ez.screen.background());
	m5.lcd.drawString(text, TFT_W - ez.theme->input_hmargin - 10, _text_cursor_y + ez.theme->input_vmargin + text_h + 10);
}

void M5ez::_drawTextInputBox(String text) {
	int16_t text_w;
	int16_t box_w = TFT_W - 2 * ez.theme->input_hmargin;
	ez.setFont(ez.theme->input_font);
	int16_t text_h = ez.fontHeight();
	_text_cursor_y = ez.canvas.top() + ez.theme->input_top + ez.theme->input_vmargin;
	_text_cursor_h = text_h;
	_text_cursor_w = m5.lcd.textWidth("A");
	m5.lcd.fillRoundRect(ez.theme->input_hmargin, ez.canvas.top() + ez.theme->input_top, box_w, text_h + ez.theme->input_vmargin * 2, 8, ez.theme->input_bgcolor);
	_text_cursor_y = ez.canvas.top() + ez.theme->input_top + ez.theme->input_vmargin;
	m5.lcd.setTextColor(ez.theme->input_fgcolor);
	String disp_text = text;
	// chop off characters from the beginning of displayed string until it fits
	while (true) {
		text_w = M5.lcd.textWidth(disp_text);
		if (text_w + _text_cursor_w > box_w) {
			disp_text = disp_text.substring(1);
		} else {
			break;
		}
	}

	// For some reason the text rendering prints a monospace font with a space at the end with a very narrow space
	if (disp_text.substring(disp_text.length() - 1) == " ") disp_text += "  ";

	m5.lcd.setTextDatum(TC_DATUM);
	m5.lcd.drawString(disp_text, TFT_W / 2 - _text_cursor_w / 2, ez.canvas.top() + ez.theme->input_top + ez.theme->input_vmargin);
	_text_cursor_x = TFT_W / 2 + text_w / 2 - _text_cursor_w / 2 + 2;
	_textCursor (true);	// draw the  cursor block
}

void M5ez::_textCursor() {
	if (ez.theme->input_cursor_blink > 0) {
		if (millis() - _text_cursor_millis > ez.theme->input_cursor_blink) {
			_textCursor(!_text_cursor_state);
		}
	}
}

void M5ez::_textCursor(bool state) {
	if (state) M5.lcd.fillRect(_text_cursor_x, _text_cursor_y, _text_cursor_w, _text_cursor_h, ez.theme->input_fgcolor);
	if (!state) M5.lcd.fillRect(_text_cursor_x, _text_cursor_y, _text_cursor_w, _text_cursor_h, ez.theme->input_bgcolor);
	_text_cursor_state = state;
	_text_cursor_millis = millis();
}

String M5ez::textBox(String header /*= ""*/, String text /*= "" */, bool readonly /*= false*/, String buttons /*= "up#Done#down"*/, const GFXfont* font /* = NULL */, uint16_t color /* = NO_COLOR */) {
	if (!font) font = ez.theme->tb_font;
	if (color == NO_COLOR) color = ez.theme->tb_color;
	if(ez.extensionControl("ezFACES", EXTENSION_CONTROL_QUERY_ENABLED, nullptr)) {
		ez.extensionControl("ezFACES", EXTENSION_CONTROL_FACES_POLL, nullptr);	// flush key buffer in FACES
	}
	else {
		readonly = true;
	}
	std::vector<line_t> lines;
	ez.screen.clear();
	uint16_t cursor_pos = text.length();
	bool cursor_state = false;
	long cursor_time = 0;
	if (header != "") ez.header.show(header);
	int8_t per_line_h = ez.fontHeight();
	String tmp_buttons = buttons;
	tmp_buttons.replace("up", "");
	tmp_buttons.replace("down", "");
	ez.buttons.show(tmp_buttons); 	//we need to draw the buttons here to make sure ez.canvas.height() is correct
	uint8_t lines_per_screen = (ez.canvas.height()) / per_line_h;
	uint8_t remainder = (ez.canvas.height()) % per_line_h;
	_wrapLines(text, ez.canvas.width() - 2 * ez.theme->tb_hmargin, lines);
	uint16_t offset = 0;
	bool redraw = true;
	ez.setFont(font);
	uint8_t cursor_width = m5.lcd.textWidth("|");
	uint8_t cursor_height = per_line_h * 0.8;
	int16_t cursor_x = 0;
	int16_t cursor_y = 0;
	while (true) {
		if (redraw) {
			if (!readonly && cursor_x && cursor_y) m5.lcd.fillRect(cursor_x, cursor_y, cursor_width, cursor_height, ez.screen.background());		//Remove current cursor
			cursor_x = cursor_y = 0;
			tmp_buttons = buttons;
			if (offset >= lines.size() - lines_per_screen) {
				tmp_buttons.replace("down", "");
			}
			if (offset <= 0) {
				offset = 0;
				tmp_buttons.replace("up", "");
			}
			ez.buttons.show(tmp_buttons);
			ez.setFont(font);
			m5.lcd.setTextColor(color, ez.screen.background());
			m5.lcd.setTextDatum(TL_DATUM);
			uint16_t x, y;
			int16_t sol, eol;
			String this_line;
			if (lines.size() > 0) {
				for (int8_t n = offset; n < offset + lines_per_screen; n++) {
					if (n < lines.size() - 1) {
						this_line = lines[n].line;
						sol = lines[n].position;
						eol = lines[n + 1].position - 1;
					} else if (n == lines.size() - 1) {
						this_line = lines[n].line;
						sol = lines[n].position;
						eol = text.length();
					} else {
						this_line = "";
						sol = -1;
						eol = text.length();
					}
					y = ez.canvas.top() + remainder * 0.7 + (n - offset) * per_line_h;
					x = ez.theme->tb_hmargin;
					if (!readonly && sol != -1 && cursor_pos >= sol && cursor_pos <= eol && n < lines.size()) { 		// if cursor is on current line
						x += m5.lcd.drawString(this_line.substring(0, cursor_pos - sol), x, y);
						cursor_x = x;
						cursor_y = y;
						x += cursor_width;
						x += m5.lcd.drawString(this_line.substring(cursor_pos - sol), x, y);
					} else {
						x += m5.lcd.drawString(this_line, x, y);
					}
					m5.lcd.fillRect(x, y, ez.canvas.width() - x, per_line_h, ez.screen.background());
				}
				redraw = false;
			}
		}
		if (!readonly && cursor_x && cursor_y && millis() - cursor_time > ez.theme->input_cursor_blink) {
			cursor_time = millis();
			if (cursor_state) {
				m5.lcd.fillRect(cursor_x, cursor_y, cursor_width, cursor_height, ez.screen.background());
				cursor_state = false;
			} else {
				m5.lcd.fillRect(cursor_x, cursor_y, cursor_width, cursor_height, color);
				cursor_state = true;
			}
		}
		String key = ez.buttons.poll();

		if(ez.extensionControl("ezFACES", EXTENSION_CONTROL_QUERY_ENABLED, nullptr)) {
			ez.extensionControl("ezFACES", EXTENSION_CONTROL_FACES_POLL, (void*)&key);
		}
		if (key == "down") {
			offset += lines_per_screen;
			ez.canvas.clear();
			redraw = true;
			key = "";
		}
		if (key == "up") {
			offset -= lines_per_screen;
			ez.canvas.clear();
			redraw = true;
			key = "";
		}
		if (key == "Done") {
			ez.screen.clear();
			return text;
		}
		if (key == (String)char(8)) {		// Delete
			if (cursor_pos > 0) {
				text = text.substring(0, cursor_pos - 1) + text.substring(cursor_pos);
				cursor_pos--;
				_wrapLines(text, ez.canvas.width() - 2 * ez.theme->tb_hmargin, lines);
				redraw = true;
			}
			key = "";
		}
		if (key == (String)char(191)) {		// left arrow on FACES keyboard
			if (cursor_pos > 0) {
				cursor_pos--;
				redraw = true;
			}
			key = "";
		}
		if (key == (String)char(193)) {		// right arrow on FACES keyboard
			if (cursor_pos < text.length()) {
				cursor_pos++;
				redraw = true;
			}
			key = "";
		}
		if (key == (String)char(183) || key == (String)char(192)) {		// up or down arrow on FACES keyboard
			uint16_t cursor_line = lines.size() - 1;
			for (uint16_t n = 0; n < lines.size(); n++) {
				if (cursor_pos < lines[n].position) {
					cursor_line = n - 1;
					break;
				}
			}
			float relative_pos = (float)(cursor_pos - lines[cursor_line].position) / lines[cursor_line].line.length();
			if (key == (String)char(183)) {		//up
				if (cursor_line >= 1) {
					cursor_pos = lines[cursor_line - 1].position + lines[cursor_line - 1].line.length() * relative_pos;
					redraw = true;
				}
			} else {	//down
				if (cursor_line < lines.size() - 1) {
					cursor_pos = lines[cursor_line + 1].position + lines[cursor_line + 1].line.length() * relative_pos;
					redraw = true;
				}
			}
			key = "";
		}

		if (!readonly) {
			if (key >= " " && key <= "~") {
				if (cursor_pos == text.length()) {
					text = text + key;
				} else {
					text = text.substring(0, cursor_pos) + key + text.substring(cursor_pos);
				}
				cursor_pos++;
				_wrapLines(text, ez.canvas.width() - 2 * ez.theme->tb_hmargin, lines);
				redraw = true;
			}
		}
	}
}

void M5ez::_wrapLines(String text, uint16_t width, std::vector<line_t>& lines) {
	lines.clear();
	int16_t offset = 0;
	int16_t last_space = 0;
	int16_t cur_space = 0;
	int16_t newline = 0;
	bool all_done = false;
	line_t new_line;

	//If there are no return chars, it's either a single line,
	//Or it's using linux/mac line endings which are a single char
	char nlchar = 13;
	if (text.indexOf(13)==-1) nlchar = 10;

	while (!all_done) {
		cur_space = text.indexOf(" ", last_space + 1);
		if (cur_space == -1) {
			cur_space = text.length();
			all_done = true;
		}
		newline = text.indexOf(char(nlchar), last_space + 1);
		if (newline != -1 && newline < cur_space) cur_space = newline;
		if (m5.lcd.textWidth(text.substring(offset, cur_space)) > width || text.substring(last_space, last_space + 1) == (String)char(nlchar)) {
			if (m5.lcd.textWidth(text.substring(offset, last_space)) <= width) {
				new_line.position = offset;
				new_line.line = text.substring(offset, last_space);
				lines.push_back(new_line);
				offset = last_space + 1;
				last_space = cur_space;
			} else {
				for (uint16_t n = offset; n < text.length(); n++) {
					if (m5.lcd.textWidth(text.substring(offset, n + 1)) > width) {
						new_line.position = offset;
						new_line.line = text.substring(offset, n);
						lines.push_back(new_line);
						offset = n;
						break;
					}
				}
			}
		} else {
			last_space = cur_space;
		}

		//Special case handle the last line
		if (all_done && offset < text.length()) {
			while(text.indexOf(char(nlchar), offset) > offset) {
				if(offset < text.length()) {
					new_line.line = text.substring(offset, text.indexOf(char(nlchar), offset));
					new_line.position = offset;
					offset = text.indexOf(char(nlchar), offset);
					lines.push_back(new_line);
				} else {
					break;
				}
			}
		}

		if (all_done && offset < text.length()) {
			new_line.position = offset;
			new_line.line = text.substring(offset);
			lines.push_back(new_line);
		}

	}
}

void M5ez::_fitLines(String text, uint16_t max_width, uint16_t min_width, std::vector<line_t>& lines) {
	uint8_t	prev_num_lines = 100;
	for (int16_t n = max_width; n > min_width; n -= 10) {
		_wrapLines(text, n, lines);
		if (lines.size() > prev_num_lines) {
			_wrapLines(text, n + 10, lines);
			return;
		}
		prev_num_lines = lines.size();
	}
}

// Some generic String object helper functions. Made public because they might be useful in user code

String M5ez::rightOf(String input, String separator, bool trim /* = true */ ) {
	input.replace("\\" + separator, (String)char(255));		// allow for backslash escaping of the separator
	int16_t sep_pos = input.indexOf(separator);
	input.replace((String)char(255), separator);
	String out = input.substring(sep_pos + 1);
	if (trim) out.trim();
	return out;
}

String M5ez::leftOf(String input, String separator, bool trim /* = true */ ) {
	input.replace("\\" + separator, (String)char(255));		// allow for backslash escaping of the separator
	int16_t sep_pos = input.indexOf(separator);
	input.replace((String)char(255), separator);
	if (sep_pos == -1) sep_pos = input.length();
	String out = input.substring(0, sep_pos);
	if (trim) out.trim();
	return out;
}

int16_t M5ez::chopString(String input, String separator, std::vector<String>& chops, bool trim /* = true */) {
	input.replace("\\" + separator, (String)char(255));		// allow for backslash escaping of the separator
	int16_t next_sep, offset = 0;
	bool done = false;
	while (!done) {
		next_sep = input.indexOf(separator, offset);
		if (next_sep == -1) {
			next_sep = input.length();
			done = true;
		}
		String chop = input.substring(offset, next_sep);
		chop.replace((String)char(255), separator);
		if (trim) chop.trim();
		chops.push_back(chop);
		offset = next_sep + 1;
	}
	return chops.size();
}

int16_t M5ez::charsFit(String input, int16_t cutoff) {
	int16_t measured;
	for (int16_t n = input.length(); n >= 0; n--) {
		measured = m5.lcd.textWidth(input.substring(0, n));
		if (measured <= cutoff) {
			return n;
		}
	}
	return 0;
}

String M5ez::clipString(String input, int16_t cutoff, bool dots /* = true */ ) {
	if (m5.lcd.textWidth(input) <= cutoff) {
		return input;
	} else {
		for (int16_t n = input.length(); n >= 0; n--) {
			String toMeasure = input.substring(0, n);
			if (dots) toMeasure = toMeasure + "..";
			if (m5.lcd.textWidth(toMeasure) <= cutoff) return toMeasure;
		}
		return "";
	}
}

bool M5ez::isBackExitOrDone(String str) {
	if (str == "back" || str == "exit" || str == "done" || str == "Back" || str == "Exit" || str == "Done") return true;
	return false;
}

// Font related m5.lcd wrappers

void M5ez::setFont(const GFXfont* font) {
	long ptrAsInt = (long) font;
	int16_t size = 1;
	if (ptrAsInt <= 16) {
 		if (ptrAsInt > 8) {
 			ptrAsInt -= 8;
 			size++;
 		}
		m5.lcd.setTextFont(ptrAsInt);
	} else {
		m5.lcd.setFreeFont(font);
	}
	m5.lcd.setTextSize(size);
}

int16_t M5ez::fontHeight() { return m5.lcd.fontHeight(m5.lcd.textfont); }

String M5ez::version() { return M5EZ_VERSION; }

bool M5ez::install(String name, extension_entry_t control) {
	extension_t ext;
	ext.name = name;
	ext.control = control;
	extensions.push_back(ext);
	return true;
}

bool M5ez::extensionControl(String name, uint8_t command, void* user) {
	for(auto& ext : M5ez::extensions) {
		if(ext.name == name)
    		return ext.control(command, user);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   M E N U
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ezMenu::ezMenu(String hdr /* = "" */) {
	_img_background = NO_COLOR;
	_offset = 0;
	_selected = -1;
	_header = hdr;
	_buttons = "";
	_font = NULL;
	_img_from_top = 0;
	_img_caption_location = TC_DATUM;
	_img_caption_font = &FreeSansBold12pt7b;
	_img_caption_color = TFT_RED;
	_img_caption_hmargin = 10;
	_img_caption_vmargin = 10;
	_sortFunction = NULL;
}

ezMenu::~ezMenu() {
	if(this == M5ez::_currentMenu) M5ez::_currentMenu = nullptr;
}

void ezMenu::txtBig() { _font = ez.theme->menu_big_font; }

void ezMenu::txtSmall() { _font = ez.theme->menu_small_font; }

void ezMenu::txtFont(const GFXfont* font) { _font = font; }

bool ezMenu::addItem(String nameAndCaption, void (*simpleFunction)() /* = NULL */, bool (*advancedFunction)(ezMenu* callingMenu) /* = NULL */, void (*drawFunction)(ezMenu* callingMenu, int16_t x, int16_t y, int16_t w, int16_t h) /* = NULL */) {
	return addItem(NULL, nameAndCaption, simpleFunction, advancedFunction, drawFunction);
}

bool ezMenu::addItem(const char *image, String nameAndCaption , void (*simpleFunction)() /* = NULL */, bool (*advancedFunction)(ezMenu* callingMenu) /* = NULL */, void (*drawFunction)(ezMenu* callingMenu, int16_t x, int16_t y, int16_t w, int16_t h) /* = NULL */) {
	MenuItem_t new_item;
	new_item.image = image;
	new_item.fs = NULL;
	new_item.nameAndCaption = nameAndCaption;
	new_item.simpleFunction = simpleFunction;
	new_item.advancedFunction = advancedFunction;
	new_item.drawFunction = drawFunction;
	if (_selected == -1) _selected = _items.size();
	_items.push_back(new_item);
	_sortItems();
	M5ez::_redraw = true;
	return true;
}

bool ezMenu::addItem(fs::FS &fs, String path, String nameAndCaption, void (*simpleFunction)() /* = NULL */, bool (*advancedFunction)(ezMenu* callingMenu) /* = NULL */, void (*drawFunction)(ezMenu* callingMenu, int16_t x, int16_t y, int16_t w, int16_t h) /* = NULL */) {
	MenuItem_t new_item;
	new_item.image = NULL;
	new_item.fs = &fs;
	new_item.path = path;
	new_item.nameAndCaption = nameAndCaption;
	new_item.simpleFunction = simpleFunction;
	new_item.advancedFunction = advancedFunction;
	new_item.drawFunction = drawFunction;
	if (_selected == -1) _selected = _items.size();
	_items.push_back(new_item);
	_sortItems();
	M5ez::_redraw = true;
	return true;
}

bool ezMenu::deleteItem(int16_t index) {
	if (index < 1 || index > _items.size()) return false;
	index--;	// internally we work with zero-referenced items
	_items.erase(_items.begin() + index);
	if (_selected >= _items.size()) _selected = _items.size() - 1;
	_fixOffset();
	M5ez::_redraw = true;
	return true;
}

bool ezMenu::deleteItem(String name) { return deleteItem(getItemNum(name)); }

String ezMenu::getTitle() { return _header; }

bool ezMenu::setCaption(int16_t index, String caption) {
	if (index < 1 || index > _items.size()) return false;
	index--;	// internally we work with zero-referenced items
	String currentName = ez.leftOf(_items[index].nameAndCaption, "|");
	String currentCaption = ez.rightOf(_items[index].nameAndCaption, "|");
	_items[index].nameAndCaption = currentName + "|" + caption;
	M5ez::_redraw = true;
	return true;
}

bool ezMenu::setCaption(String name, String caption) { return setCaption(getItemNum(name), caption); }

int16_t ezMenu::getItemNum(String name) {
	String itemName;
	for (int16_t n = 0; n < _items.size(); n++) {
		itemName = ez.leftOf(_items[n].nameAndCaption, "|");
		if (itemName == name) return n + 1;
	}
	return 0;
}

void ezMenu::setSortFunction(bool (*sortFunction)(const char* s1, const char* s2)) {
	_sortFunction = sortFunction;
	_sortItems();	// In case the menu is already populated
}

void ezMenu::buttons(String bttns) {
	_buttons = bttns;
}

void ezMenu::upOnFirst(String nameAndCaption) {
	_up_on_first = nameAndCaption;
}

void ezMenu::leftOnFirst(String nameAndCaption) {
	_up_on_first = nameAndCaption;
}

void ezMenu::downOnLast(String nameAndCaption) {
	_down_on_last = nameAndCaption;
}

void ezMenu::rightOnLast(String nameAndCaption) {
	_down_on_last = nameAndCaption;
}

void ezMenu::run() {
	while (runOnce()) {}
}

int16_t ezMenu::runOnce() {
	int16_t result;
	M5ez::_currentMenu = this;
	if(_items.size() == 0) return 0;
	if (_selected == -1) _selected = 0;
	if (!_font)	_font = ez.theme->menu_big_font;	// Cannot be in constructor: ez.theme not there yet
	for (int16_t n = 0; n < _items.size(); n++) {
		if (_items[n].image != NULL || _items[n].fs != NULL) {
			result = _runImagesOnce();
			if(0 == result) M5ez::_currentMenu = nullptr;
			return result;
		}
	}
	result = _runTextOnce();
	if(0 == result) M5ez::_currentMenu = nullptr;
	return result;
}

int16_t ezMenu::_runTextOnce() {
	if (_buttons == "") _buttons = "up # select # down";
	ez.screen.clear();
	if (_header != "") ez.header.show(_header);
	ez.setFont(_font);
	_per_item_h = ez.fontHeight();
	ez.buttons.show(_buttons); 	//we need to draw the buttons here to make sure ez.canvas.height() is correct
	_items_per_screen = (ez.canvas.height() - 5) / _per_item_h;
	_drawItems();
	while (true) {
		int16_t old_selected = _selected;
		int16_t old_offset = _offset;
		String tmp_buttons = _buttons;
		if (_selected <= 0) tmp_buttons.replace("up", _up_on_first); 
		if (_selected >= _items.size() - 1) tmp_buttons.replace("down", _down_on_last); 
		ez.buttons.show(tmp_buttons);
		String name = ez.leftOf(_items[_selected].nameAndCaption, "|");
		String pressed;
		while (true) {
			pressed = ez.buttons.poll();
			if (M5ez::_redraw) _drawItems();
			if (pressed != "") break;
		}
		if (pressed == "up") {
			_selected--;
			_fixOffset();
		} else if (pressed == "down") {
			_selected++;
			_fixOffset();
		} else if (pressed == "first") {
			_selected = 0;
			_offset = 0;
		} else if (pressed == "last") {
			_selected = _items.size() - 1;
			_offset = 0;
			_fixOffset();
		} else if ( (ez.isBackExitOrDone(name) && !_items[_selected].advancedFunction) || ez.isBackExitOrDone(pressed) ) {
			_pick_button = pressed;
			_selected = -1;
			ez.screen.clear();
			return 0;
		} else {
			// Some other key must have been pressed. We're done here!
			ez.screen.clear();
			_pick_button = pressed;
			if (_items[_selected].simpleFunction) {
				(_items[_selected].simpleFunction)();
			}
			if (_items[_selected].advancedFunction) {
				if (!(_items[_selected].advancedFunction)(this)) return 0;
			}
			return _selected + 1; 	// We return items starting at one, but work starting at zero internally
		}

		// Flicker prevention, only redraw whole menu if scrolled
		if (_offset == old_offset) {
			int16_t top_item_h = ez.canvas.top() + (ez.canvas.height() % _per_item_h) / 2;   // remainder of screen left over by last item not fitting split to center menu
			if (_items[old_selected].drawFunction) {
				ez.setFont(_font);
				(_items[old_selected].drawFunction)(this, ez.theme->menu_lmargin, top_item_h + (old_selected - _offset) * _per_item_h, TFT_W - ez.theme->menu_lmargin - ez.theme->menu_rmargin, _per_item_h);
			} else {
				_drawItem(old_selected - _offset, ez.rightOf(_items[old_selected].nameAndCaption, "|"), false);
			};
			if (_items[_selected].drawFunction) {
				ez.setFont(_font);
				(_items[_selected].drawFunction)(this, ez.theme->menu_lmargin, top_item_h + (_selected - _offset) * _per_item_h, TFT_W - ez.theme->menu_lmargin - ez.theme->menu_rmargin, _per_item_h);
			} else {
				_drawItem(_selected - _offset, ez.rightOf(_items[_selected].nameAndCaption, "|"), true);
			};
		} else {
			ez.canvas.clear();
			_drawItems();
		}
	}
}

void ezMenu::_drawItems() {
	for (int16_t n = 0; n < _items_per_screen; n++) {
		int16_t item_ref = _offset + n;
		if (item_ref < _items.size()) {
			if (_items[item_ref].drawFunction) {
				int16_t top_item_h = ez.canvas.top() + (ez.canvas.height() % _per_item_h) / 2;   // remainder of screen left over by last item not fitting split to center menu
				ez.setFont(_font);
				(_items[item_ref].drawFunction)(this, ez.theme->menu_lmargin, top_item_h + n * _per_item_h, TFT_W - ez.theme->menu_lmargin - ez.theme->menu_rmargin, _per_item_h);
			} else {
				_drawItem(n, ez.rightOf(_items[item_ref].nameAndCaption, "|"), (item_ref == _selected));
			}
		}
	}
	_Arrows();
	M5ez::_redraw = false;
}

void ezMenu::_drawItem(int16_t n, String text, bool selected) {
	uint16_t fill_color;
	ez.setFont(_font);
	int16_t top_item_h = ez.canvas.top() + (ez.canvas.height() % _per_item_h) / 2;   // remainder of screen left over by last item not fitting split to center menu
	m5.lcd.setTextDatum(CL_DATUM);
	if (selected) {
		fill_color = ez.theme->menu_sel_bgcolor;
		m5.lcd.setTextColor(ez.theme->menu_sel_fgcolor);
	} else {
		fill_color = ez.screen.background();
		m5.lcd.setTextColor(ez.theme->menu_item_color);
	}
	text = ez.clipString(text, TFT_W - ez.theme->menu_lmargin - 2 * ez.theme->menu_item_hmargin - ez.theme->menu_rmargin);
	m5.lcd.fillRoundRect(ez.theme->menu_lmargin, top_item_h + n * _per_item_h, TFT_W - ez.theme->menu_lmargin - ez.theme->menu_rmargin, _per_item_h, ez.theme->menu_item_radius, fill_color);
	m5.lcd.drawString(ez.leftOf(text, "\t"), ez.theme->menu_lmargin + ez.theme->menu_item_hmargin, top_item_h + _per_item_h / 2 + n * _per_item_h - 2);
	if (text.indexOf("\t") != -1) {
		m5.lcd.setTextDatum(CR_DATUM);
		m5.lcd.drawString(ez.rightOf(text, "\t"),  TFT_W - ez.theme->menu_rmargin - ez.theme->menu_item_hmargin, top_item_h + _per_item_h / 2 + n * _per_item_h - 2);
	}
}

void ezMenu::imgBackground(uint16_t color) {
	_img_background = color;
}

void ezMenu::imgFromTop(int16_t offset) { _img_from_top = offset; }

void ezMenu::imgCaptionFont(const GFXfont* font) { _img_caption_font = font; }

void ezMenu::imgCaptionLocation(uint8_t datum) { _img_caption_location = datum; }

void ezMenu::imgCaptionColor(uint16_t color) { _img_caption_color = color; }

void ezMenu::imgCaptionMargins(int16_t hmargin, int16_t vmargin) {
	_img_caption_hmargin = hmargin;
	_img_caption_vmargin = vmargin;
}

void ezMenu::imgCaptionMargins(int16_t margin) {
	_img_caption_hmargin = margin;
	_img_caption_vmargin = margin;
}

int16_t ezMenu::_runImagesOnce() {
	if (_buttons == "") _buttons = "left # select # right";
	if (_img_background == NO_COLOR) _img_background = ez.theme->background;
	ez.screen.clear(_img_background);
	String tmp_buttons = _buttons;
	tmp_buttons.replace("left", "");
	tmp_buttons.replace("right", "");
	ez.buttons.show(tmp_buttons);
	ez.screen.clear(_img_background);
	if (_header != "") ez.header.show(_header);
	_drawImage(_items[_selected]);
	_drawCaption();
	while (true) {
		tmp_buttons = _buttons;
		if (_selected <= 0) tmp_buttons.replace("left", "");
		if (_selected >= _items.size() - 1) tmp_buttons.replace("right", "");
		ez.buttons.show(tmp_buttons);
		String name = ez.leftOf(_items[_selected].nameAndCaption, "|");
		String pressed;
		while (true) {
			pressed = ez.buttons.poll();
			if (pressed != "") break;
		}
		if (pressed == "left") {
			_selected--;
			ez.canvas.clear();
			_drawImage(_items[_selected]);
			_drawCaption();
		} else if (pressed == "right") {
			_selected++;
			ez.canvas.clear();
			_drawImage(_items[_selected]);
			_drawCaption();
		} else if ( (ez.isBackExitOrDone(name) && !_items[_selected].advancedFunction) || ez.isBackExitOrDone(pressed) ) {
			_pick_button = pressed;
			_selected = -1;
			ez.screen.clear();
			return 0;
		} else {
			// Some other key must have been pressed. We're done here!
			ez.screen.clear();
			_pick_button = pressed;
			if (_items[_selected].simpleFunction != NULL) {
				(_items[_selected].simpleFunction)();
				ez.screen.clear();
			}
			if (_items[_selected].advancedFunction != NULL) {
				if (!(_items[_selected].advancedFunction)(this)) {
					ez.screen.clear();
					return 0;
				} else {
					ez.screen.clear();
				}
			}
			return _selected + 1; 	// We return items starting at one, but work starting at zero internally
		}
	}
}

void ezMenu::_drawImage(MenuItem_t &item) {
	if (item.image) {
		m5.lcd.drawJpg((uint8_t *)item.image, (sizeof(item.image) / sizeof(item.image[0])), 0, ez.canvas.top() + _img_from_top, TFT_W, ez.canvas.height() - _img_from_top);
	}
	if (item.fs) {
		m5.lcd.drawJpgFile(*(item.fs), item.path.c_str(), 0, ez.canvas.top() + _img_from_top, TFT_W, ez.canvas.height() - _img_from_top);
	}
}

void ezMenu::_drawCaption() {
	int16_t x, y;
	String caption = ez.rightOf(_items[_selected].nameAndCaption, "|");
	if (_img_caption_font == NULL || caption == "") return;
	ez.setFont(_img_caption_font);
	m5.lcd.setTextColor(_img_caption_color);
	m5.lcd.setTextDatum(_img_caption_location);
	// Set X and Y for printing caption seperately, less code duplication
	switch(_img_caption_location) {
		case TL_DATUM:
		case ML_DATUM:
		case BL_DATUM:
			x = ez.canvas.left() + _img_caption_hmargin;
			break;
		case TC_DATUM:
		case MC_DATUM:
		case BC_DATUM:
			x = ez.canvas.left() + ez.canvas.width() / 2;
			break;
		case TR_DATUM:
		case MR_DATUM:
		case BR_DATUM:
		default:
			x = ez.canvas.right() - _img_caption_hmargin;
			break;
			return;
		//
	}
	switch(_img_caption_location) {
		case TL_DATUM:
		case TC_DATUM:
		case TR_DATUM:
			y = ez.canvas.top() + _img_caption_vmargin;
			break;
		case ML_DATUM:
		case MC_DATUM:
		case MR_DATUM:
			y = ez.canvas.top() + ez.canvas.height() / 2;
			break;
		case BL_DATUM:
		case BC_DATUM:
		case BR_DATUM:
		default:
			y = ez.canvas.bottom() - _img_caption_vmargin;
			break;
		//
	}
	m5.lcd.drawString(caption, x, y);
}

void ezMenu::_fixOffset() {
	if (_selected != -1) {

		if (_selected >= _offset + _items_per_screen) _offset = _selected - _items_per_screen + 1;
		if (_selected < _offset) _offset = _selected;

		// If there's stuff above the current screen and screen is not full (can only happen after deleteItem)
		if (_offset > 0 && _offset + _items_per_screen > _items.size()) {
			_offset = _items.size() - _items_per_screen;
		}

	} else {
		_offset = 0;
	}
}

int16_t ezMenu::pick() { return _selected + 1; }

String ezMenu::pickName() { return ez.leftOf(_items[_selected].nameAndCaption, "|"); }

String ezMenu::pickCaption() { return ez.rightOf(_items[_selected].nameAndCaption, "|"); }

String ezMenu::pickButton() { return _pick_button; }

void ezMenu::_Arrows() {
	uint16_t fill_color;

	int16_t top = ez.canvas.top();
	int16_t height = ez.canvas.height();

	// Up arrow
	if (_offset > 0) {
		fill_color = ez.theme->menu_item_color;
	} else {
		fill_color = ez.screen.background();
	}
	static uint8_t l = ez.theme->menu_arrows_lmargin;
	m5.lcd.fillTriangle(l, top + 25, l + 10, top + 25, l + 5, top + 10, fill_color);

	// Down arrow
	if (_items.size() > _offset + _items_per_screen) {
		fill_color = ez.theme->menu_item_color;
	} else {
		fill_color = ez.screen.background();
	}
	m5.lcd.fillTriangle(l, top + height - 25, l + 10, top + height - 25, l + 5, top + height - 10, fill_color);
}

bool ezMenu::_sortWrapper(MenuItem_t& item1, MenuItem_t& item2) {
	// Be sure to set _sortFunction before calling _sortWrapper(), as _sortItems ensures.
	return _sortFunction(item1.nameAndCaption.c_str(), item2.nameAndCaption.c_str());
}

void ezMenu::_sortItems() {
	if(_sortFunction && _items.size() > 1) {
		using namespace std::placeholders;	// For _1, _2
		sort(_items.begin(), _items.end(), std::bind(&ezMenu::_sortWrapper, this, _1, _2));
		M5ez::_redraw = true;
	}
}

// For sorting by Names as quickly as possible
bool ezMenu::sort_asc_name_cs (const char* s1, const char* s2) { return 0 >     strcmp(s1, s2); }
bool ezMenu::sort_asc_name_ci (const char* s1, const char* s2) { return 0 > strcasecmp(s1, s2); }
bool ezMenu::sort_dsc_name_cs (const char* s1, const char* s2) { return 0 <     strcmp(s1, s2); }
bool ezMenu::sort_dsc_name_ci (const char* s1, const char* s2) { return 0 < strcasecmp(s1, s2); }

// For sorting by Caption if there is one, falling back to sorting by Name if no Caption is provided (all purpose)
const char* captionSortHelper(const char* nameAndCaption) {
	char* sub = strchr(nameAndCaption, '|');	// Find the separator
	if(nullptr == sub) return nameAndCaption;	// If none, return the entire string
	sub++;                          			// move past the separator
	while(isspace(sub[0])) sub++;				// trim whitespace
	return sub;
}
bool ezMenu::sort_asc_caption_cs (const char* s1, const char* s2) { return 0 >     strcmp(captionSortHelper(s1), captionSortHelper(s2)); }
bool ezMenu::sort_asc_caption_ci (const char* s1, const char* s2) { return 0 > strcasecmp(captionSortHelper(s1), captionSortHelper(s2)); }
bool ezMenu::sort_dsc_caption_cs (const char* s1, const char* s2) { return 0 <     strcmp(captionSortHelper(s1), captionSortHelper(s2)); }
bool ezMenu::sort_dsc_caption_ci (const char* s1, const char* s2) { return 0 < strcasecmp(captionSortHelper(s1), captionSortHelper(s2)); }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   P R O G R E S S B A R
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ezProgressBar::ezProgressBar(String header /* = "" */, String msg /* = "" */, String buttons /* = "" */, const GFXfont* font /* = NULL */, uint16_t color /* = NO_COLOR */, uint16_t bar_color /* = NO_COLOR */, bool show_val /* = false */, uint16_t val_color /* = NO_COLOR */) {
	if (!font) font = ez.theme->msg_font;
	if (color == NO_COLOR) color = ez.theme->msg_color;
	if (bar_color == NO_COLOR) bar_color = ez.theme->progressbar_color;
	if (val_color == NO_COLOR) val_color = ez.theme->progressbar_val_color;
	_bar_color = bar_color;
	_show_val = show_val;
	_val_color = val_color;
	_old_val = -1;
	ez.screen.clear();
	if (header != "") ez.header.show(header);
	ez.buttons.show(buttons);
	std::vector<line_t> lines;
	msg.replace("|", (String)char(13));
	m5.lcd.setTextDatum(CC_DATUM);
	m5.lcd.setTextColor(color);
	ez.setFont(font);
	ez._fitLines(msg, ez.canvas.width() - 2 * ez.theme->msg_hmargin, ez.canvas.width() / 3, lines);
	uint8_t font_h = ez.fontHeight();
	uint8_t num_lines = lines.size() + 2;
	for (uint8_t n = 0; n < lines.size(); n++) {
		int16_t y = ez.canvas.top() + ez.canvas.height() / 2 - ( (num_lines - 1) * font_h / 2) + n * font_h;
		m5.lcd.drawString(lines[n].line, TFT_W / 2, y);
	}
	_bar_y = ez.canvas.top() + ez.canvas.height() / 2 + ( (num_lines - 1) * font_h / 2) - ez.theme->progressbar_width / 2;
	for (uint8_t n = 0; n < ez.theme->progressbar_line_width; n++) {
		m5.lcd.drawRect(ez.canvas.left() + ez.theme->msg_hmargin + n, _bar_y + n, ez.canvas.width() - 2 * ez.theme->msg_hmargin - 2 * n, ez.theme->progressbar_width - 2 * n, bar_color);
	}
}

void ezProgressBar::value(float val) {
	// Prevent flickering
	if (_old_val == val) {
		return;
	}
	_old_val = val;

	uint16_t left = ez.canvas.left() + ez.theme->msg_hmargin + ez.theme->progressbar_line_width;
	uint16_t width = (int16_t)(ez.canvas.width() - 2 * ez.theme->msg_hmargin - 2 * ez.theme->progressbar_line_width);
	m5.lcd.fillRect(left, _bar_y + ez.theme->progressbar_line_width, width * val / 100, ez.theme->progressbar_width - 2 * ez.theme->progressbar_line_width, _bar_color);
	m5.lcd.fillRect(left + (width * val / 100), _bar_y + ez.theme->progressbar_line_width, width - (width * val / 100), ez.theme->progressbar_width - 2 * ez.theme->progressbar_line_width, ez.screen.background());

	if (_show_val == true) {
		m5.lcd.setTextDatum(CC_DATUM);
		m5.lcd.setTextColor(_val_color);
		ez.setFont(ez.theme->msg_font);
		m5.lcd.drawFloat(val, 0, TFT_W / 2, _bar_y + ez.theme->progressbar_width / 2 - 1);
	}
}


M5ez ez;
