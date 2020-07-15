#include <M5ez.h>

#include <Preferences.h>

#ifdef M5EZ_WIFI
	#include <WiFi.h>
	extern "C" {
		#include "esp_wifi.h"
		#include "esp_wps.h"
	}
	#include <WiFiClientSecure.h>		// For ez.update
	#include <Update.h>
#endif //M5EZ_WIFI

#ifdef M5EZ_CLOCK
	#include <ezTime.h>
#endif

#ifdef M5EZ_BLE
	#include <BLEDevice.h>
	#include <BLEUtils.h>
	#include <BLEScan.h>
	#include <BLEAdvertisedDevice.h>
#endif

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
//	if (_shown)	{
		return _title;
//	} else {
//		return "";
//	}
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
	#ifdef M5EZ_BACKLIGHT
		if (keystr != "") ez.backlight.activity();
	#endif
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
	#ifdef M5EZ_WIFI
		ez.wifi.begin();
	#endif
	#ifdef M5EZ_BLE
		ez.ble.begin();
	#endif
	#ifdef M5EZ_BATTERY
		ez.battery.begin();
	#endif
	#ifdef M5EZ_CLOCK
		ez.clock.begin();
	#endif
	#ifdef M5EZ_BACKLIGHT
		ez.backlight.begin();
	#endif
	#ifdef M5EZ_FACES
		ez.faces.begin();
	#endif	
	if (ez.themes.size() > 1) {
		ez.settings.menuObj.addItem("Theme chooser", ez.theme->menu);
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
//   B A C K L I G H T
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_BACKLIGHT
	uint8_t ezBacklight::_brightness;
	uint8_t ezBacklight::_inactivity;
	uint32_t ezBacklight::_last_activity;
	bool ezBacklight::_backlight_off = false;

	void ezBacklight::begin() {
		ez.addEvent(ez.backlight.loop);
		ez.settings.menuObj.addItem("Backlight settings", ez.backlight.menu);
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
				//
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
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   C L O C K
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_CLOCK
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
		ez.settings.menuObj.addItem("Clock settings", ez.clock.menu);
		ez.addEvent(ez.clock.loop);
		ez.clock.restart();
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
			ez.header.insert(RIGHTMOST, "clock", width, ez.clock.draw);
		}
	}
	
	void ezClock::menu() {
		bool on_orig = _on;
		bool clock12_orig = _clock12;
		bool am_pm_orig = _am_pm;
		String tz_orig = _timezone;
		while(true) {
			ezMenu clockmenu("Clock settings");
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
					ez.clock.restart();
					break;
				case 2:
					_timezone = ez.textInput("Enter timezone");
					if (_timezone == "") _timezone = "GeoIP";
					if (tz.setLocation(_timezone)) _timezone = tz.getOlsen();
					break;
				case 3:
					_clock12 = !_clock12;
					ez.clock.restart();
					break;
				case 4:
					_am_pm = !_am_pm;
					ez.clock.restart();
					break;
				case 0:
					if (_am_pm != am_pm_orig || _clock12 != clock12_orig || _on != on_orig || _timezone != tz_orig) {
						_writePrefs();
					}
					return;
				//
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
			
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   F A C E S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_FACES

	bool ezFACES::_on;

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
		ez.settings.menuObj.addItem("FACES keyboard", ez.faces.menu);
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
			#ifdef M5EZ_BACKLIGHT
				ez.backlight.activity();
			#endif
			return out;
		}
		return "";
	}
	
	bool ezFACES::on() {
		return _on;
	}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   W I F I
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_WIFI

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

	void ezWifi::begin() {
		#ifdef M5EZ_WIFI_DEBUG
			Serial.println("EZWIFI: Initialising");
		#endif
		WiFi.mode(WIFI_MODE_STA);
		WiFi.setAutoConnect(false);		// We have our own multi-AP version of this
		WiFi.setAutoReconnect(false);	// So we turn off the ESP32's versions
		WiFi.setHostname("M5Stack");
		ez.wifi.readFlash();
		_state = EZWIFI_IDLE;
		const uint8_t cutoffs[] = { 0, 20, 40, 70 };
		ez.settings.menuObj.addItem("Wifi settings", ez.wifi.menu);
		ez.header.insert(RIGHTMOST, "wifi", sizeof(cutoffs) * (ez.theme->signal_bar_width + ez.theme->signal_bar_gap) + 2 * ez.theme->header_hmargin, ez.wifi._drawWidget);
		ez.addEvent(ez.wifi.loop);
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
		ez.wifi.writeFlash();
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
				ez.wifi.remove(callingMenu->pick() - 1);
				callingMenu->deleteItem(callingMenu->pick());
				ez.wifi.writeFlash();
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
			ez.wifi.add(WiFi.SSID(), WiFi.psk());
			ez.wifi.writeFlash();
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
				if (millis() < _wait_until) return 250;
			case EZWIFI_IDLE:
				#ifdef M5EZ_WIFI_DEBUG
					Serial.println("EZWIFI: Starting scan");
				#endif
				WiFi.mode(WIFI_MODE_STA);
				WiFi.scanNetworks(true);
				_current_from_scan = 0;
				_state = EZWIFI_SCANNING;
				_wait_until = millis() + 10000;
				break;
			case EZWIFI_SCANNING:
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
				if (millis() > _wait_until) {
					#ifdef M5EZ_WIFI_DEBUG
						Serial.println("EZWIFI: Connect timed out...");
					#endif
					WiFi.disconnect();
					_current_from_scan++;
					_state = EZWIFI_SCANNING;
				}
			case EZWIFI_AUTOCONNECT_DISABLED:
				return 250;
			default:
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
	

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   B L E
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_BLE

	class M5ezClientCallback : public BLEClientCallbacks {
		void onConnect(BLEClient*) {}
		void onDisconnect(BLEClient*) {
			ez.ble._cleanup();
		}
	} _bleClientCallback;

	// https://www.bluetooth.com/specifications/gatt/services/
	const std::vector<std::pair<uint16_t, String>> ezBLE::_gattUuids = {
		{ 0x1800, "Generic Access" }, 
		{ 0x1811, "Alert Notification Service" }, 
		{ 0x1815, "Automation IO" }, 
		{ 0x180F, "Battery Service" }, 
		{ 0x183B, "Binary Sensor" }, 
		{ 0x1810, "Blood Pressure" }, 
		{ 0x181B, "Body Composition" }, 
		{ 0x181E, "Bond Management Service" }, 
		{ 0x181F, "Continuous Glucose Monitoring" }, 
		{ 0x1805, "Current Time Service" }, 
		{ 0x1818, "Cycling Power" }, 
		{ 0x1816, "Cycling Speed and Cadence" }, 
		{ 0x180A, "Device Information" }, 
		{ 0x183C, "Emergency Configuration" }, 
		{ 0x181A, "Environmental Sensing" }, 
		{ 0x1826, "Fitness Machine" }, 
		{ 0x1801, "Generic Attribute" }, 
		{ 0x1808, "Glucose" }, 
		{ 0x1809, "Health Thermometer" }, 
		{ 0x180D, "Heart Rate" }, 
		{ 0x1823, "HTTP Proxy" }, 
		{ 0x1812, "Human Interface Device" }, 
		{ 0x1802, "Immediate Alert" }, 
		{ 0x1821, "Indoor Positioning" }, 
		{ 0x183A, "Insulin Delivery" }, 
		{ 0x1820, "Internet Protocol Support Service" }, 
		{ 0x1803, "Link Loss" }, 
		{ 0x1819, "Location and Navigation" }, 
		{ 0x1827, "Mesh Provisioning Service" }, 
		{ 0x1828, "Mesh Proxy Service" }, 
		{ 0x1807, "Next DST Change Service" }, 
		{ 0x1825, "Object Transfer Service" }, 
		{ 0x180E, "Phone Alert Status Service" }, 
		{ 0x1822, "Pulse Oximeter Service" }, 
		{ 0x1829, "Reconnection Configuration" }, 
		{ 0x1806, "Reference Time Update Service" }, 
		{ 0x1814, "Running Speed and Cadence" }, 
		{ 0x1813, "Scan Parameters" }, 
		{ 0x1824, "Transport Discovery" }, 
		{ 0x1804, "Tx Power" }, 
		{ 0x181C, "User Data" }, 
		{ 0x181D, "Weight Scale" }, 
	};

	bool ezBLE::_on = false;
	bool ezBLE::_initialized = false;
	std::vector<BLEClient*> ezBLE::_clients;

	void ezBLE::begin() {
		ez.ble.readFlash();
		ez.settings.menuObj.addItem("BLE settings", ez.ble.menu);
		if (_on) {
			_refresh();
		}
	}

	void ezBLE::readFlash() {
		Preferences prefs;
		prefs.begin("M5ez", true);	// true: read-only
		_on = prefs.getBool("ble_on", false);
		prefs.end();
	}

	void ezBLE::writeFlash() {
		Preferences prefs;
		prefs.begin("M5ez");
		prefs.putBool("ble_on", _on);
		prefs.end();
	}

	void ezBLE::menu() {
		bool on_orig = _on;
		while(true) {
			ezMenu blemenu("BLE Settings");
			blemenu.txtSmall();
			blemenu.addItem("on|Turn On\t" + (String)(_on ? "on" : "off"));
			if (_on) {
				blemenu.addItem("Scan and connect", NULL, _scan);
				blemenu.addItem("Clients", NULL, _listClients);
			}
			blemenu.buttons("up#Back#select##down#");
			switch (blemenu.runOnce()) {
				case 1:
					_on = !_on;
					_refresh();
					break;
				case 0:
					if (_on != on_orig) {
						writeFlash();
					}
					return;
			}
		}
	}

	void ezBLE::disconnect() {
		for (auto& client : _clients) {
			if (client->isConnected())
				client->disconnect();
		}
		_clients.clear();
	}

	BLEClient* ezBLE::getClient(uint16_t index) {
		if (index >= _clients.size())
			return nullptr;
		return _clients[index];
	}

	uint16_t ezBLE::getClientCount() {
		return static_cast<uint16_t>(_clients.size());
	}

	bool ezBLE::_scan(ezMenu* callingMenu) {
		BLEScan* bleScan = BLEDevice::getScan(); //create new scan
		bleScan->setActiveScan(true); //active scan uses more power, but get results faster
		bleScan->setInterval(100);
		bleScan->setWindow(99);  // less or equal setInterval value
		ez.msgBox("Bluetooth", "Scanning ...", "");
		BLEScanResults foundDevices = bleScan->start(5, false);
		ezMenu devicesmenu("Found " + String(foundDevices.getCount()) + " devices");
		devicesmenu.txtSmall();
		for (int i = 0; i < foundDevices.getCount(); i++) {
			const char* name = nullptr;
			if (foundDevices.getDevice(i).getName().size() == 0)
				name = foundDevices.getDevice(i).getAddress().toString().c_str();
			else
				name = foundDevices.getDevice(i).getName().c_str();
			devicesmenu.addItem(name);
		}
		devicesmenu.buttons("up#Back#select##down#");
		if (devicesmenu.runOnce()) {
			BLEAdvertisedDevice device = foundDevices.getDevice(devicesmenu.pick() - 1);
			_connect(device);
		}
		bleScan->clearResults();
		return true;
	}

	void ezBLE::_connect(class BLEAdvertisedDevice& device) {
		ez.msgBox("Bluetooth", "Connecting ...", "");
		BLEClient* client = BLEDevice::createClient();
		client->setClientCallbacks(&_bleClientCallback);
		if (client->connect(&device)) {
			_clients.push_back(client);
			ez.msgBox("Bluetooth", "Device connected.");
		} else {
			delete client;
			ez.msgBox("Bluetooth", "Connection failed!");
		}
	}

	bool ezBLE::_listClients(ezMenu* callingMenu) {
		ezMenu clientsmenu("Clients");
		clientsmenu.txtSmall();
		for (auto& client : _clients) {
			clientsmenu.addItem(client->getPeerAddress().toString().c_str());
		}
		clientsmenu.buttons("up#Back#select##down#");
		while (clientsmenu.runOnce()) {
			if (!_showClient(_clients[clientsmenu.pick() - 1]))
				return false;
		}
		return true;
	}

	bool ezBLE::_showClient(BLEClient* client) {
		ezMenu clientmenu(String(client->getPeerAddress().toString().c_str()));
		clientmenu.txtSmall();
		clientmenu.buttons("up#Back#select##down#");
		auto& services = *client->getServices();
		for (auto& service : services) {
			bool found = false;
			for (auto& pair : _gattUuids) {
				if (BLEUUID(pair.first).equals(service.second->getUUID())) {
					clientmenu.addItem(pair.second);
					found = true;
					break;
				}
			}
			if (!found)
				clientmenu.addItem(service.first.c_str());
		}
		clientmenu.addItem("Disconnect");
		while (clientmenu.runOnce()) {
			if (clientmenu.pickName() == "Disconnect") {
				client->disconnect();
				return false;
			}
		}
		return true;
	}

	void ezBLE::_cleanup() {
		if (_clients.size() > 0) {
			_clients.erase(std::remove_if(_clients.begin(), _clients.end(), [](BLEClient* client) {
				bool shouldRemove = !client->isConnected();
				if (shouldRemove)
					delete client;
				return shouldRemove;
			}), _clients.end());
		}
	}

	void ezBLE::_refresh() {
		if (_on) {
			if (!_initialized) {
				_initialized = true;
				BLEDevice::init(M5EZ_BLE_DEVICE_NAME);
			}
		} else {
			disconnect();
			if (_initialized) {
				_initialized = false;
				_cleanup();
				BLEDevice::deinit(false);
			}
		}
	}

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   B A T T E R Y
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_BATTERY
	bool ezBattery::_on = false;

	void ezBattery::begin() {
		Wire.begin();
		ez.battery.readFlash();
		ez.settings.menuObj.addItem("Battery settings", ez.battery.menu);
		if (_on) {
			_refresh();
		}
	}

	void ezBattery::readFlash() {
		Preferences prefs;
		prefs.begin("M5ez", true);	// true: read-only
		_on = prefs.getBool("battery_icon_on", false);
		prefs.end();
	}

	void ezBattery::writeFlash() {
		Preferences prefs;
		prefs.begin("M5ez");
		prefs.putBool("battery_icon_on", _on);
		prefs.end();
	}

	void ezBattery::menu() {
		bool on_orig = _on;
		while(true) {
			ezMenu clockmenu("Battery settings");
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
						writeFlash();
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

	//Transform the M5Stack built in battery level into an internal format.
	// From [100, 75, 50, 25, 0] to [4, 3, 2, 1, 0]
	uint8_t ezBattery::getTransformedBatteryLevel()
	{
		switch (m5.Power.getBatteryLevel()) 
		{
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
	uint32_t ezBattery::getBatteryBarColor(uint8_t batteryLevel)
	{
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
			ez.header.insert(RIGHTMOST, "battery", ez.theme->battery_bar_width + 2 * ez.theme->header_hmargin, ez.battery._drawWidget);
			ez.addEvent(ez.battery.loop);
		} else {
			ez.header.remove("battery");
			ez.removeEvent(ez.battery.loop);
		}
	}

	void ezBattery::_drawWidget(uint16_t x, uint16_t w) {
		uint8_t currentBatteryLevel = getTransformedBatteryLevel();
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
				bar_width, bar_height, getBatteryBarColor(currentBatteryLevel));
		}
	}

#endif


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
#ifdef M5EZ_WIFI
	ezWifi M5ez::wifi;
	constexpr ezWifi& M5ez::w;
#endif	
#ifdef M5EZ_BLE
	ezBLE M5ez::ble;
#endif
#ifdef M5EZ_BATTERY
	ezBattery M5ez::battery;
#endif
#ifdef M5EZ_BACKLIGHT
	ezBacklight M5ez::backlight;
#endif
#ifdef M5EZ_CLOCK
	ezClock M5ez::clock;
#endif
#ifdef M5EZ_FACES
	ezFACES M5ez::faces;
#endif	
std::vector<event_t> M5ez::_events;

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
	for (uint8_t n = 0; n< _events.size(); n++) {
		if (millis() > _events[n].when) {
			uint16_t r = (_events[n].function)();
			if (r) {
				_events[n].when = millis() + r - 1;
			} else {
				_events.erase(_events.begin() + n);
				break;		// make sure we don't go beyond _events.size() after deletion
			}
		}
	}
#ifdef M5EZ_CLOCK
	events();		//TMP	
#endif
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
	"KB14|!?:;\\#$^&#SP#KB15|*()_-+=\|#Del#KB0|More#LCK:SYM|Lock#KB16|'\"`@%\\/#KB17|<>{}[]()#Done",	//KB13
	"!#?#:#;#\\##$#^#&#Back",															//KB14
	"*#(#)#_#-#+#=#\\|#Back",															//KB15
	"'#\"#`#@#%#/#\##Back",																//KB16
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
	#ifdef M5EZ_FACES
		if (ez.faces.on()) {
			current_kb = locked_kb = prev_kb = ez.theme->input_faces_btns;
			ez.faces.poll(); 	// flush key buffer in FACES
		}
	#endif
	String tmpstr;	
	String text = defaultText;
	ez.screen.clear();
	if (header != "") ez.header.show(header);
	_drawTextInputBox(text);
	String key;
	ez.buttons.show(_keydefs[current_kb]);

	while (true) {
		key = ez.buttons.poll();
		#ifdef M5EZ_FACES
			if (ez.faces.on() && key == "") key = ez.faces.poll();
		#endif
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
	#ifdef M5EZ_FACES
		if (ez.faces.on()) {
			ez.faces.poll(); 	// flush key buffer in FACES
		} else {
			readonly = true;
		}
	#endif
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
		#ifdef M5EZ_FACES
			if (ez.faces.on() && key == "") key = ez.faces.poll();
		#endif
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
	if(_items.size() == 0) return 0;
	if (_selected == -1) _selected = 0;
	if (!_font)	_font = ez.theme->menu_big_font;	// Cannot be in constructor: ez.theme not there yet
	for (int16_t n = 0; n < _items.size(); n++) {
		if (_items[n].image != NULL || _items[n].fs != NULL) return _runImagesOnce();
	}
	return _runTextOnce();
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

ezProgressBar::ezProgressBar(String header /* = "" */, String msg /* = "" */, String buttons /* = "" */, const GFXfont* font /* = NULL */, uint16_t color /* = NO_COLOR */, uint16_t bar_color /* = NO_COLOR */) {
	if (!font) font = ez.theme->msg_font;
	if (color == NO_COLOR) color = ez.theme->msg_color;
	if (bar_color == NO_COLOR) bar_color = ez.theme->progressbar_color;
	_bar_color = bar_color;
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
	uint16_t left = ez.canvas.left() + ez.theme->msg_hmargin + ez.theme->progressbar_line_width;
	uint16_t width = (int16_t)(ez.canvas.width() - 2 * ez.theme->msg_hmargin - 2 * ez.theme->progressbar_line_width);
	m5.lcd.fillRect(left, _bar_y + ez.theme->progressbar_line_width, width * val / 100, ez.theme->progressbar_width - 2 * ez.theme->progressbar_line_width, _bar_color);
	m5.lcd.fillRect(left + (width * val / 100), _bar_y + ez.theme->progressbar_line_width, width - (width * val / 100), ez.theme->progressbar_width - 2 * ez.theme->progressbar_line_width, ez.screen.background());
}



M5ez ez;
