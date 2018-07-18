#include <M5Stack.h>

#include <WiFi.h>
extern "C" {
#include "esp_wifi.h"
#include "esp_wps.h"
}

#include <M5ez.h>


WiFiEvent_t _WPS_event;
String _WPS_pin;
bool _WPS_new_event;


M5ez::M5ez() {
	//m5.begin();
	_background = SCREEN_BGCOLOR;
	_print_wrap = true;

//	_print_scroll = true;

//	nvs_flash_init();
	esp_wifi_start();
	delay(200);
	WiFi.getMode();
	WiFi.mode(WIFI_STA);
//	WiFi.begin();
	WiFi.disconnect();
//	WiFi.setHostname("M5Stack");

}

// Screen, canvas, etc...

void M5ez::background(uint16_t color) {	_background = color; }

uint16_t M5ez::background() { return _background; }

void M5ez::clearScreen() { clearScreen(_background); }

void M5ez::clearScreen(uint16_t color) {
	m5.lcd.fillRect(0, 0, TFT_W, TFT_H, color);
	_btn_a_s = _btn_a_l = _btn_b_s = _btn_b_l = _btn_c_s = _btn_c_l = "";
	_btn_ab = _btn_bc = _btn_ac = "";
	_lower_button_row = false;
	_upper_button_row = false;
	_header_displayed = false;
	m5.lcd.setTextColor(EP_COLOR);
	_setCanvasVars();
	_print_x = _print_y = _print_lmargin = 0;
	_print_font = EP_FONT;
	_print_color = EP_COLOR;
}

void M5ez::clearCanvas() {
	m5.lcd.fillRect(0, _canvas_t, TFT_W, _canvas_h, _background);
	m5.lcd.setTextColor(TFT_BLACK, _background);
}

int16_t M5ez::canvasTop() { return _canvas_t; }

int16_t M5ez::canvasBottom() { return _canvas_t + _canvas_h - 1; }

int16_t M5ez::canvasLeft() { return 0; }

int16_t M5ez::canvasRight() { return TFT_W - 1; }

int16_t M5ez::canvasWidth() { return TFT_W; }

int16_t M5ez::canvasHeight() { return _canvas_h; }

void M5ez::_setCanvasVars() {
	int16_t button_rows = _upper_button_row ? 2 : (_lower_button_row ? 1 : 0);
	_canvas_t = (_header_displayed ? HEADER_HEIGHT : 0);
	_canvas_h = TFT_H - _canvas_t - (button_rows * (BUTTON_HEIGHT + BUTTON_GAP) ); 
}


// Header

void M5ez::drawHeader(String header_txt) {
	m5.lcd.fillRect(0, 0, TFT_W, HEADER_HEIGHT, HEADER_BGCOLOR);
	m5.lcd.setTextDatum(TL_DATUM);
	m5.lcd.setTextColor(HEADER_FGCOLOR);
	ez.setFont(HEADER_FONT);
	m5.lcd.drawString(header_txt, HEADER_HMARGIN, HEADER_TMARGIN);
	_header_displayed = true;
	_wifiSignalBars(true);
	_setCanvasVars();
	if (_print_y == 0) _print_y = _canvas_t;
}

void M5ez::removeHeader() {
	if (_header_displayed) {
		m5.lcd.fillRect(0, 0, TFT_W, HEADER_HEIGHT, _background);
		_header_displayed = false;
		_setCanvasVars();
	}
}


// Buttons

void M5ez::drawButtons(String buttons) {
	buttons.trim();
	// number of elements is one more than number of separators
	int16_t num = countStringInString(buttons, "#") + 1;
	if (num == 1) {
		// One element, so center button only
		_drawButtons("", "", buttons, "", "", "", "", "", "");
	} else {
		String ButtonArray[num];
		chopStringIntoArray(buttons, "#", ButtonArray, true);
		if (num == 3) {
			// Three elements, so shortpress only
			_drawButtons(ButtonArray[0], "", ButtonArray[1], "", ButtonArray[2], "", "", "", "");
		}
		if (num == 6) {
			// Six elements, so all buttons long and short
			_drawButtons(ButtonArray[0], ButtonArray[1], ButtonArray[2], ButtonArray[3], ButtonArray[4], ButtonArray[5], "", "", "");
		}
		if (num == 9) {
			// Nine elements, so all buttons long and short plus the top row of three multi-keys
			_drawButtons(ButtonArray[0], ButtonArray[1], ButtonArray[2], ButtonArray[3], ButtonArray[4], ButtonArray[5], ButtonArray[6], ButtonArray[7], ButtonArray[8]);
		}
	}
}

void M5ez::_drawButtons(String btn_a_s, String btn_a_l, String btn_b_s, String btn_b_l, String btn_c_s, String btn_c_l, String btn_ab, String btn_bc, String btn_ac) {
	int16_t btnwidth = int16_t( (TFT_W - 4 * BUTTON_GAP ) / 3);

	// See if any buttons are used on the bottom row
	if (btn_a_s != "" || btn_a_l != "" || btn_b_s != "" || btn_b_l != "" || btn_c_s != "" || btn_c_l != "") {
		if (!_lower_button_row) {
			// If the lower button row wasn't there before, clear the area first
			m5.lcd.fillRect(0, TFT_H - BUTTON_HEIGHT - BUTTON_GAP, TFT_W, BUTTON_HEIGHT + BUTTON_GAP, _background);
		}
		// Then draw the three buttons there. (drawButton erases single buttons if unused.)
		if (_btn_a_s != btn_a_s || _btn_a_l != btn_a_l) {
			_drawButton(1, rightOf(btn_a_s, "|", true), rightOf(btn_a_l, "|", true), BUTTON_GAP, btnwidth);
			_btn_a_s = btn_a_s;
			_btn_a_l = btn_a_l;
		}
		if (_btn_b_s != btn_b_s || _btn_b_l != btn_b_l) {
			_drawButton(1, rightOf(btn_b_s, "|", true), rightOf(btn_b_l, "|", true), btnwidth + 2 * BUTTON_GAP, btnwidth);
			_btn_b_s = btn_b_s;
			_btn_b_l = btn_b_l;		
		}
		if (_btn_c_s != btn_c_s || _btn_c_l != btn_c_l) {
			_drawButton(1, rightOf(btn_c_s, "|", true), rightOf(btn_c_l, "|", true), 2 * btnwidth + 3 * BUTTON_GAP, btnwidth);
			_btn_c_s = btn_c_s;
			_btn_c_l = btn_c_l;		
		}
		_lower_button_row = true;
	} else {
		if (_lower_button_row) {
			// If there was a lower button row before and it's now gone, clear the area
			m5.lcd.fillRect(0, TFT_H - BUTTON_HEIGHT - BUTTON_GAP, TFT_W, BUTTON_HEIGHT + BUTTON_GAP, _background);
			_btn_a_s = _btn_a_l = _btn_b_s = _btn_b_l = _btn_c_s = _btn_c_l = "";
			_lower_button_row = false;
		}
	}

	//Now check if there are multi-buttons used (top row)
	if (btn_ab != "" || btn_bc != "" || btn_ac != "") {
		if (!_upper_button_row) {
			// If the upper button row wasn't there before, clear the area first
			m5.lcd.fillRect(0, TFT_H - 2 * (BUTTON_HEIGHT + BUTTON_GAP), TFT_W, BUTTON_HEIGHT + BUTTON_GAP, _background);
		}
		// Then draw the buttons
		if (_btn_ab != btn_ab) {
			_drawButton(2, rightOf(btn_ab, "|", true), "", BUTTON_GAP + (btnwidth / 2), btnwidth);
			_btn_ab = btn_ab;
		}
		if (_btn_bc != btn_bc) {
			_drawButton(2, rightOf(btn_bc, "|", true), "", (2 * BUTTON_GAP) + btnwidth + (btnwidth / 2), btnwidth);
			_btn_bc = btn_bc;
		}
		if (_btn_ac != btn_ac) {
			// Two halves of the same button

			// ugly in code, prettier on the screen: making the buttons square on the screen edges to signal wrap-around
			m5.lcd.fillRect(0, TFT_H - 2 * BUTTON_HEIGHT - BUTTON_GAP, btnwidth / 4, BUTTON_HEIGHT, BUTTON_BGCOLOR_T);
			m5.lcd.fillRect(TFT_W - (btnwidth / 4), TFT_H - 2 * BUTTON_HEIGHT - BUTTON_GAP, btnwidth / 4, BUTTON_HEIGHT, BUTTON_BGCOLOR_T);

			_drawButton(2, rightOf(btn_ac, "|", true), "", (3 * BUTTON_GAP) + (2 * btnwidth) + (btnwidth / 2), (btnwidth / 2));
			_drawButton(2, rightOf(btn_ac, "|", true), "", 0, (btnwidth / 2));

			_btn_ac = btn_ac;
		}
		_upper_button_row = true;
	} else {
		if (_upper_button_row) {
			// If there was an upper button row before and it's now gone, clear the area
			m5.lcd.fillRect(0, TFT_H - 2 * (BUTTON_HEIGHT + BUTTON_GAP), TFT_W, BUTTON_HEIGHT + BUTTON_GAP, _background);
			_btn_ab = _btn_bc = _btn_ac = "";
			_upper_button_row = false;
		}			
	}
	_setCanvasVars();
}

void M5ez::_drawButton(int16_t row, String text_s, String text_l, int16_t x, int16_t w) {
	// row = 1 for lower and 2 for upper row
	int16_t y, bg_color;
	if (row == 1) {
		y = TFT_H - BUTTON_HEIGHT;
		bg_color = BUTTON_BGCOLOR_B;
	}
	if (row == 2){
		y = TFT_H - 2 * BUTTON_HEIGHT - BUTTON_GAP;
		bg_color = BUTTON_BGCOLOR_T;
	}
	if (text_s != "" || text_l != "") {
		ez.setFont(BUTTON_FONT);
		m5.lcd.fillRoundRect(x, y, w, BUTTON_HEIGHT, BUTTON_RADIUS, bg_color);
		if (text_l != "") {
			_drawButtonString(text_s, x + BUTTON_HMARGIN, y + BUTTON_TMARGIN, BUTTON_FGCOLOR, TL_DATUM);
		} else {
			_drawButtonString(text_s, x + int16_t(w / 2), y + BUTTON_TMARGIN, BUTTON_FGCOLOR, TC_DATUM);
		}
		_drawButtonString(text_l, x + w - BUTTON_HMARGIN, y + BUTTON_TMARGIN, BUTTON_LONGCOLOR, TR_DATUM);
	} else {
		m5.lcd.fillRect(x, y, w, BUTTON_HEIGHT, _background);
	}
}

void M5ez::_drawButtonString(String text, int16_t x, int16_t y, uint16_t color, int16_t datum) {
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

String M5ez::getButtons() {
	String keystr = "";
	M5.update();
	_wifiSignalBars();

	if (!_key_release_wait) {
		if (_btn_ab != "" && m5.BtnA.isPressed() && m5.BtnB.isPressed() ) {
			keystr = leftOf(_btn_ab, "|", true);
			_key_release_wait = true;
		}
		if (_btn_bc != "" && m5.BtnB.isPressed() && m5.BtnC.isPressed() ) {
			keystr = leftOf(_btn_bc, "|", true);
			_key_release_wait = true;
		}
		if (_btn_ac != "" && m5.BtnA.isPressed() && m5.BtnC.isPressed() ) {
			keystr = leftOf(_btn_ac, "|", true);
			_key_release_wait = true;
		}

		if (_btn_a_l != "" && m5.BtnA.pressedFor(LONGPRESS_TIME) ) {
			keystr = leftOf(_btn_a_l, "|", true);
			_key_release_wait = true;
		}
		if (_btn_a_s != "" && m5.BtnA.wasReleased() ) {
			keystr = leftOf(_btn_a_s, "|", true);
		}

		if (_btn_b_l != "" && m5.BtnB.pressedFor(LONGPRESS_TIME) ) {
			keystr = leftOf(_btn_b_l, "|", true);
			_key_release_wait = true;
		}
		if (_btn_b_s != "" && m5.BtnB.wasReleased() ) {
			keystr = leftOf(_btn_b_s, "|", true);
		}

		if (_btn_c_l != "" && m5.BtnC.pressedFor(LONGPRESS_TIME) ) {
			keystr = leftOf(_btn_c_l, "|", true);
			_key_release_wait = true;
		}
		if (_btn_c_s != "" && m5.BtnC.wasReleased() ) {
			keystr = leftOf(_btn_c_s, "|", true);
		}
	}

	if (m5.BtnA.isReleased() && m5.BtnB.isReleased() && m5.BtnC.isReleased() ) {
		_key_release_wait = false;
	}		

	if (keystr == "~") keystr = "";
	return keystr;
}

String M5ez::waitForButtons() {
	String keystr = "";
	while (keystr == "") {
		keystr = getButtons();
	}
	return keystr;
}

String M5ez::waitForButtons(String buttons) {
	drawButtons(buttons);
	return waitForButtons();
}


// ez.msgBox

String M5ez::msgBox(String header, String msg, String buttons /* = "OK" */, const bool blocking /* = true */, const GFXfont* font /* = MSG_FONT */, uint16_t color /* = MSG_COLOR */) {
	clearScreen();
	if (header != "") drawHeader(header);
	drawButtons(buttons);
	int16_t num_lines = countStringInString(msg, "|") + 1;
	String lines[num_lines];
	chopStringIntoArray(msg, "|", lines);
	m5.lcd.setTextDatum(CC_DATUM);
	m5.lcd.setTextColor(color);
	ez.setFont(font);
	int16_t font_h = ez.fontHeight();
	for (int16_t n = 0; n < num_lines; n++) {
		int16_t y = _canvas_t + _canvas_h / 2 - ( (num_lines - 1) * font_h / 2) + n * font_h;
		m5.lcd.drawString(lines[n], TFT_W / 2, y);
	}
	if (buttons != "" && blocking) {
		String ret = waitForButtons();
		clearScreen();
		return ret;
	} else {
		return "";
	}
}


// ez.textInput

String M5ez::textInput(String header /* = "" */, String defaultText /* = "" */) {

	int16_t current_kb = 0, prev_kb = 0, locked_kb = 0;
	String tmpstr;	
	String text = defaultText;
	clearScreen();
	if (header != "") drawHeader(header);
	_drawTextInputBox(text);
	drawButtons(_keydefs[current_kb]);

	while (true) {
		String key = getButtons();
	
		if (key == "Done") return text;
		if (key.substring(0, 2) == "KB") {
			prev_kb = current_kb;
			tmpstr = key.substring(2);
			current_kb = tmpstr.toInt();
			drawButtons(_keydefs[current_kb]);
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
				drawButtons(_keydefs[current_kb]);
			}
			key = "";
		}
		if (key == "Back") {
			current_kb = prev_kb;
			drawButtons(_keydefs[current_kb]);
			key = "";
		}
		if (key == "Del") {
			text = text.substring(0, text.length() - 1);
			_drawTextInputBox(text);
			key = "";
		}
		if (key == "SP") key = " ";
		if (key != "") {
			current_kb = locked_kb;
			drawButtons(_keydefs[current_kb]);
			text += key;
			_drawTextInputBox(text);
		}
	
		_textCursor();
	}
}

void M5ez::_drawTextInputLockString(String text) {
	m5.lcd.setTextColor(TFT_RED);
	ez.setFont(INPUT_KEYLOCK_FONT);
	m5.lcd.setTextDatum(TR_DATUM);
	int16_t text_h = ez.fontHeight();
	m5.lcd.fillRect(0, _text_cursor_y + INPUT_VMARGIN + 10 + text_h, TFT_W, text_h, _background);
	m5.lcd.drawString(text, TFT_W - INPUT_HMARGIN - 10, _text_cursor_y + INPUT_VMARGIN + text_h + 10);
}

void M5ez::_drawTextInputBox(String text) {
	int16_t text_w;
	int16_t box_w = TFT_W - 2 * INPUT_HMARGIN;
	ez.setFont(INPUT_FONT);
	int16_t text_h = ez.fontHeight();
	_text_cursor_y = _canvas_t + INPUT_TOP + INPUT_VMARGIN;
	_text_cursor_h = text_h;
	_text_cursor_w = m5.lcd.textWidth("A");
	m5.lcd.fillRoundRect(INPUT_HMARGIN, _canvas_t + INPUT_TOP, box_w, text_h + INPUT_VMARGIN * 2, 8, INPUT_BGCOLOR);
	_text_cursor_y = _canvas_t + INPUT_TOP + INPUT_VMARGIN;
	m5.lcd.setTextColor(INPUT_FGCOLOR);
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
	m5.lcd.drawString(disp_text, TFT_W / 2 - _text_cursor_w / 2, _canvas_t + INPUT_TOP + INPUT_VMARGIN);
	_text_cursor_x = TFT_W / 2 + text_w / 2 - _text_cursor_w / 2 + 2;
	_textCursor (true);	// draw the  cursor block
}

void M5ez::_textCursor() {
	if (INPUT_CURSOR_BLINK > 0) {
		if (millis() - _text_cursor_millis > INPUT_CURSOR_BLINK) { 
			_textCursor(!_text_cursor_state);
		}
	}
}

void M5ez::_textCursor(bool state) {
	if (state) M5.lcd.fillRect(_text_cursor_x, _text_cursor_y, _text_cursor_w, _text_cursor_h, INPUT_FGCOLOR);
	if (!state) M5.lcd.fillRect(_text_cursor_x, _text_cursor_y, _text_cursor_w, _text_cursor_h, INPUT_BGCOLOR);
	_text_cursor_state = state;
	_text_cursor_millis = millis();
}


// ez.print

void M5ez::print(String text, int16_t x /* = -1 */, int16_t y /* = -1 */, const GFXfont* font /* = NULL */, uint16_t color /* = TFT_TRANSPARENT */) {
	//Serial.println ("print - x:" + String(x) + " y:" + String(y) + " text:" + text);
	if (font == NULL) font = _print_font;
	if (color == TFT_TRANSPARENT) color = _print_color;
	if (x == -1) x = _print_x;
	if (y == -1) y = _print_y;
	m5.lcd.setTextDatum(TL_DATUM);
	m5.lcd.setTextColor(color);
	ez.setFont(font);
	int16_t h = ez.fontHeight();

//	Scrolling not supported until we get m5.lcd.readRect(...) to work

// 	if (_print_scroll && y + h > _canvas_t + _canvas_h) {
// 		scrollCanvas(h);
// 		y = _print_y;
// 	}

	if (y + h <= _canvas_t + _canvas_h) {
		if (_print_wrap && m5.lcd.textWidth(text) > TFT_W - x) {	// Doesn't fit, needs truncation or wrap
			//Serial.println("doesn't fit");
			String this_fits;
			for (int16_t n = text.length(); n > 0; n--) {
				this_fits = text.substring(0, n - 1);
				Serial.println("New try: " + this_fits);
				if (m5.lcd.textWidth(this_fits) <= TFT_W - _print_x) {
					println (this_fits, x, y, font, color);
					print(text.substring(n - 1));	//if it still doesn't fit, the next iteration of print will deal with it
					break;
				}
			}
		} else {
			m5.lcd.drawString(text, x, y);
			_print_x = x + m5.lcd.textWidth(text);
			_print_y = y;
		}
	}
}

void M5ez::println(String text, int16_t x /* = -1 */, int16_t y /* = -1 */, const GFXfont* font /* = NULL */, uint16_t color /* = TFT_TRANSPARENT */) {
	//Serial.println ("println - x:" + String(x) + " y:" + String(y) + " text:" + text);
	print(text, x, y, font, color);
	_print_x = _print_lmargin;
	_print_y = _print_y + ez.fontHeight();
}

void M5ez::printFont(const GFXfont* font) { _print_font = font; }

void M5ez::printColor(uint16_t color) { _print_color = color; }

void M5ez::printLmargin(int16_t lmargin) { 
	_print_lmargin = lmargin;
	if (_print_x < lmargin) _print_x = lmargin; 	
}

void M5ez::printWrap(bool state) { _print_wrap = state; }


//Scroll not supported until we get m5.lcd.readRect(...) to work
/*
void M5ez::printScroll(bool state) { _print_scroll = state; }

void M5ez::scrollCanvas(int pixels) {
	uint16_t buffer[TFT_W];

	int16_t m;
	for (m = 0; m < 320; m++) {
		buffer[m] = 0x1F00;  // blue
	}

	for (int16_t n = _canvas_t; n < _canvas_t + _canvas_h - pixels; n++) {
		m5.lcd.readRect(0, n + pixels, TFT_W, 1, buffer);
		
	for (m = 0; m < 320; m++) {
		Serial.print(String(buffer[m]) + "-");
	}
		

		m5.lcd.pushRect(0, n, TFT_W, 1, buffer);
	}
	_print_y = _print_y - pixels;
}
*/

void M5ez::wifiStatus() {
	while (true) {
		clearScreen();
		drawHeader("WiFi settings");
		printFont(&FreeSans9pt7b);
		printLmargin(10);
		println("");
		if (WiFi.isConnected() ) {
			print("Status:"); println("Connected", 140);
			print("SSID:"); println(WiFi.SSID(), 140);
			print("Key:"); println(WiFi.psk(), 140);
			print("My IP"); println(WiFi.localIP().toString(), 140);
			print("My hostname:"); println(WiFi.getHostname(), 140);
			print("Router IP:"); println(WiFi.gatewayIP().toString(), 140);
			print("Router BSSID:"); println(WiFi.BSSIDstr(), 140);
			drawButtons("#Disconnect#Back");
		} else {
			print("Status:"); println("Disconnected", 140);
			drawButtons("#Join#Back");		
		}
		String pressed = waitForButtons();
		if (pressed == "Back") return;
		if (pressed == "Join") wifiJoin();
		if (pressed == "Disconnect") {
			WiFi.disconnect();
			while(WiFi.isConnected()) {}
		}
	}
}

void M5ez::wifiJoin() {
	String SSID = "", key = "";
	ezMenu joinmenu("Joining a network");
	joinmenu.addItem("Scan and join");
	joinmenu.addItem("SmartConfig");
	joinmenu.addItem("WPS Button");
	joinmenu.addItem("WPS Pin Code");
	joinmenu.addItem("Back");
	while(!WiFi.isConnected() && joinmenu.runOnce()) {
	
		if (joinmenu.pickName() == "Scan and join") {
			msgBox("WiFi setup menu", "Scanning ...", "");
			WiFi.disconnect();
			delay(100);
			Serial.println("Scan start");
			int16_t n = WiFi.scanNetworks();
			Serial.println("Scan done");
			if (n == 0) {
				msgBox("WiFi setup menu", "No networks found", "OK");
			} else {
				ezMenu networks("Select your netork");
				networks.txtSmall();
				for (int16_t i = 0; i < n; ++i) {
					// No duplicates (multiple BSSIDs on same SSID)
					// because we're only picking an SSID here
					if (!networks.getItemNum(WiFi.SSID(i))) {
						networks.addItem(WiFi.SSID(i));
					}
				}
				networks.runOnce();
				if (networks.pick()) {
 					SSID = networks.pickName();
  					if ( WiFi.encryptionType(networks.pick() - 1) == WIFI_AUTH_OPEN) {
  						WiFi.begin(SSID.c_str());
					} else {
						key = textInput("Enter the WEP key");
						WiFi.begin(SSID.c_str(), key.c_str());
					}
				}
				msgBox("WiFi setup menu", "Connecting ...", "Abort", false);
				String button;
				int16_t status, prev_status;
				while (button = getButtons()) {
					if (button == "Abort") {
						WiFi.disconnect();
						break;
					}
					status = WiFi.status();
					if (status != prev_status) {
						Serial.println("Wifi status: " + String(status));
						prev_status = status;
					}
					if (status == WL_CONNECTED) {
						break;
					}
					if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
						msgBox("WiFi setup menu", "Connect failed. | | (Wrong password?)", "OK");
						break;
					}
				}	
				WiFi.scanDelete();
			}
		}
		
		if (joinmenu.pickName() == "SmartConfig") {
			msgBox("SmartConfig setup", "Waiting for SmartConfig", "Abort", false);
			WiFi.beginSmartConfig();			
			bool done_already = false;
			while (!WiFi.isConnected()) {
				if (getButtons() == "Abort") {
					WiFi.stopSmartConfig();
					break;
				}
				if (WiFi.smartConfigDone() && !done_already) {
					msgBox("SmartConfig setup", "SmartConfig received | Connecting ...", "Abort", false);
					done_already = true;
				}
			}
		}
		
		if (joinmenu.pickName().substring(0,3) == "WPS") {
			msgBox("WPS setup", "Waiting for WPS", "Abort", false);
			esp_wps_config_t config;
			if (joinmenu.pickName() == "WPS Button") {
				config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);
			} else {
				config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PIN);
			}
			WiFi.onEvent(_WPShelper);
		    esp_wifi_wps_enable(&config);
    		esp_wifi_wps_start(0);
    		
			_WPS_new_event = false;
			while (!WiFi.isConnected()) {
				if (getButtons() == "Abort") {
					esp_wifi_wps_disable();
					break;
				}
				switch(_WPS_event && _WPS_new_event) {
					case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
						msgBox("WPS setup", "WPS successful | Connecting ...", "Abort", false);
						esp_wifi_wps_disable();
       					delay(10);
        				WiFi.begin();
        				break;
        			case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        			case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        				msgBox("WPS setup", "WPS failed or timed out | Retrying ...", "Abort", false);
						esp_wifi_wps_disable();
						esp_wifi_wps_enable(&config);
						esp_wifi_wps_start(0);
						break;
					case SYSTEM_EVENT_STA_WPS_ER_PIN:
						msgBox("WPS setup", "WPS PIN: " + _WPS_pin, "Abort", false);
						break;
				}
				_WPS_new_event = false;
			}
		}
	}
} 
						
void _WPShelper(WiFiEvent_t event, system_event_info_t info) {
	_WPS_event = event;
	_WPS_new_event = true;
	if (event == SYSTEM_EVENT_STA_WPS_ER_PIN) {
	    char wps_pin[9];
	    for (int16_t i = 0; i < 8; i++) {
	        wps_pin[i] = info.sta_er_pin.pin_code[i];
    	}
	    wps_pin[8] = '\0';
		_WPS_pin = String(wps_pin);
	}
}

void M5ez::_wifiSignalBars(bool now /* = false */) {
	if (millis() - _last_wifi_signal_update < 3000 && !now) return;	// Update not more than once per 3 sec
	_last_wifi_signal_update = millis();
	if (!_header_displayed) return;									// No header, no signal bars
	const uint8_t bar_width = 4;
	const uint8_t bar_gap = 2;
	const uint8_t cutoffs[] = { 0, 25, 50, 75 };
	uint8_t max_bars = sizeof(cutoffs);
	uint8_t bars;
	uint16_t left_offset = TFT_W - HEADER_HMARGIN - max_bars * (bar_width + bar_gap);
	if (WiFi.isConnected()) {
		uint8_t signal = map(100 + WiFi.RSSI(), 5, 90, 0, 100);
		for (uint8_t n = 0; n < sizeof(cutoffs); n++) {				// Determine how many bars signal is.
			if (signal >= cutoffs[n]) bars = n + 1;
		}
	} else {
		bars = 0;
	}
	uint8_t top = HEADER_HEIGHT / 10;
	uint8_t max_len = HEADER_HEIGHT * 0.8;
	uint8_t this_len;
	for (uint8_t n = 0; n < max_bars; n++) {
		this_len = ((float) (n + 1) / max_bars) * max_len;
		m5.lcd.fillRect(left_offset + n * (bar_width + bar_gap), top + max_len - this_len, bar_width, this_len, (n + 1 <= bars ? HEADER_FGCOLOR : HEADER_BGCOLOR) );
	}
}
	
	
	
	

// Some generic String object helper functions. Made public because they might be useful in user code

String M5ez::rightOf(String input, String separator, bool trim /* = true */ ) {
	int16_t i;
	for (i = 0; i < input.length() - separator.length() + 1; i++) {
		if (input.substring(i, i + separator.length()) == separator) {
			String o = input.substring(i + separator.length() );
			if (trim) o.trim();
			return o;
		}
	}
	return input;
}

String M5ez::leftOf(String input, String separator, bool trim /* = true */) {
	int16_t i;
	for (i = 0; i < input.length() - separator.length() + 1; i++) {
		if (input.substring(i, i + separator.length()) == separator) {
			String o = input.substring(0, i);
			if (trim) o.trim();
			return o;
		}
	}
	return input;
}

int16_t M5ez::countStringInString(String haystack, String needle) {
	int16_t count = 0, i;
	for (i = 0; i < haystack.length() - needle.length() + 1; i++) {
		if (haystack.substring(i, i + needle.length()) == needle) count++;
	}
	return count;
}

int16_t M5ez::chopStringIntoArray(String input, String separator, String array[], bool trim /* = true */) {
	int16_t i, last_separator_end = 0, count = 0;
	for (i = 0; i < input.length() - separator.length() + 1; i++) {
		if (input.substring(i, i + separator.length()) == separator) {
			array[count] = input.substring(last_separator_end, i);
			if (trim) array[count].trim();
			count++;
			last_separator_end = i + separator.length();
		}
	}
	array[count] = input.substring(last_separator_end);
	if (trim) array[count].trim();
	return count;
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
			if (dots) toMeasure = toMeasure + "...";
			int16_t measured = m5.lcd.textWidth(input.substring(0, n) + "...");
			if (m5.lcd.textWidth(toMeasure) <= cutoff) {
				if (dots) {
					return toMeasure + "..." ;
				} else {
					return toMeasure;
				}
			}
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

	


//
// ezMenu class
//

ezMenu::ezMenu(String hdr /* = "" */) {
	_offset = 0;
	_selected = -1;
	_header = hdr;
	_buttons = "";
	_font = MENU_BIG_FONT;
	_old_background = ez.background();
	_img_from_top = 0;
}

ezMenu::~ezMenu() {
	ez.background(_old_background);
}

void ezMenu::txtBig() { _font = MENU_BIG_FONT; }

void ezMenu::txtSmall() { _font = MENU_SMALL_FONT; }

void ezMenu::txtFont(const GFXfont* font) { _font = font; }

bool ezMenu::addItem(String nameAndCaption, void (*simpleFunction)() /* = NULL */, bool (*advancedFunction)(ezMenu* callingMenu) /* = NULL */) {
	addItem(NULL, nameAndCaption, simpleFunction, advancedFunction);
}

bool ezMenu::addItem(const char *image, String nameAndCaption , void (*simpleFunction)() /* = NULL */, bool (*advancedFunction)(ezMenu* callingMenu) /* = NULL */) {
	MenuItem_t new_item;
	new_item.image = image;
	new_item.nameAndCaption = nameAndCaption;
	new_item.simpleFunction = simpleFunction;
	new_item.advancedFunction = advancedFunction;
	if (_selected == -1) _selected = _items.size();
	_items.push_back(new_item);
	return true;
}

bool ezMenu::deleteItem(int16_t index) {
	if (index < 1 || index > _items.size()) return false;
	index--;	// internally we work with zero-referenced items
	_items.erase(_items.begin() + index);
	if (_selected >= _items.size()) _selected = _items.size() - 1;
	_fixOffset();
	return true;
}

bool ezMenu::deleteItem(String name) { return deleteItem(getItemNum(name)); }

int16_t ezMenu::getItemNum(String name) {
	String itemName;
	for (int16_t n = 0; n < _items.size(); n++) {
		itemName = ez.leftOf(_items[n].nameAndCaption, "|");
		if (itemName == name) return n + 1;
	}
	return 0;
}

void ezMenu::buttons(String bttns) {
	_buttons = bttns;
}	

void ezMenu::run() {
	while (runOnce()) {}
}

int16_t ezMenu::runOnce() {
	for (int16_t n = 0; n < _items.size(); n++) {
		if (_items[n].image != NULL) return _runImagesOnce();
	}
	return _runTextOnce();
}

int16_t ezMenu::_runTextOnce() {
	if (_selected == -1 || _items.size() == 0) return 0;
	if (_buttons == "") _buttons = "up # select # down";
	ez.clearScreen();
	if (_header != "") ez.drawHeader(_header);
	ez.setFont(_font);
	_per_item_h = ez.fontHeight();
	ez.drawButtons(_buttons); 	//we need to draw the buttons here to make sure canvasHeight() is correct
	_items_per_screen = (ez.canvasHeight() - 5) / _per_item_h;
	_drawItems();
	while (true) {
		int16_t old_selected = _selected;
		int16_t old_offset = _offset;
		String tmp_buttons = _buttons;
		if (_selected <= 0) tmp_buttons.replace("up", ""); 
		if (_selected >= _items.size() - 1) tmp_buttons.replace("down", ""); 
		ez.drawButtons(tmp_buttons);
		String name = ez.leftOf(_items[_selected].nameAndCaption, "|");
		String pressed = ez.waitForButtons();
		if (pressed == "up") {
			_selected--;
			_fixOffset();
		} else if (pressed == "down") {
			_selected++;
			_fixOffset();
		} else if ( (ez.isBackExitOrDone(name) && !_items[_selected].advancedFunction) || ez.isBackExitOrDone(pressed) ) {
			_pick_button = pressed;
			_selected = -1;
			ez.clearScreen();
			return 0;
		} else {
			// Some other key must have been pressed. We're done here!
			ez.clearScreen();
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
			_drawItem(old_selected - _offset, ez.rightOf(_items[old_selected].nameAndCaption, "|"), false);
			_drawItem(_selected - _offset, ez.rightOf(_items[_selected].nameAndCaption, "|"), true);
		} else {
			ez.clearCanvas();
			_drawItems();
		}
	}			
}

void ezMenu::_drawItems() {	
	for (int16_t n = 0; n < _items_per_screen; n++) {
		int16_t item_ref = _offset + n;
		if (item_ref < _items.size()) {
			_drawItem(n, ez.rightOf(_items[item_ref].nameAndCaption, "|"), (item_ref == _selected));
		}
	}
	_Arrows();					
}

void ezMenu::_drawItem(int16_t n, String text, bool selected) {
	uint16_t fill_color;
	ez.setFont(_font);
	int16_t font_h = ez.fontHeight();
	int16_t top_item_h = ez.canvasTop() + (ez.canvasHeight() % _per_item_h) / 2;   // remainder of screen left over by last item not fitting split to center menu
	m5.lcd.setTextDatum(CL_DATUM);
	if (selected) {
		fill_color = MENU_SEL_BGCOLOR;
		m5.lcd.setTextColor(MENU_SEL_FGCOLOR);
	} else {
		fill_color = ez.background();
		m5.lcd.setTextColor(MENU_ITEM_COLOR);
	}
	int16_t max_width = TFT_W - MENU_LMARGIN - 1.3 * MENU_ITEM_HMARGIN - MENU_RMARGIN;
	text = text.substring(0, ez.charsFit(text, max_width));
	m5.lcd.fillRoundRect(MENU_LMARGIN, top_item_h + n * _per_item_h, TFT_W - MENU_LMARGIN - MENU_RMARGIN, _per_item_h, MENU_ITEM_RADIUS, fill_color);
	m5.lcd.drawString(text, MENU_LMARGIN + MENU_ITEM_HMARGIN, top_item_h + _per_item_h / 2 + n * _per_item_h - 2);
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
	ez.background(_img_background);
	if (_selected == -1 || _items.size() == 0) return 0;
	if (_buttons == "") _buttons = "left # select # right";
	String tmp_buttons = _buttons;
	tmp_buttons.replace("left", ""); 
	tmp_buttons.replace("right", "");
	ez.drawButtons(tmp_buttons);
	ez.clearScreen();
	if (_header != "") ez.drawHeader(_header);
	_drawImage(_items[_selected].image);
	_drawCaption();
	while (true) {
		int16_t old_selected = _selected;
		tmp_buttons = _buttons;
		if (_selected <= 0) tmp_buttons.replace("left", ""); 
		if (_selected >= _items.size() - 1) tmp_buttons.replace("right", ""); 
		ez.drawButtons(tmp_buttons);
		String name = ez.leftOf(_items[_selected].nameAndCaption, "|");
		String pressed = ez.waitForButtons();
		if (pressed == "left") {
			_selected--;
			ez.clearCanvas();
			_drawImage(_items[_selected].image);
			_drawCaption();
		} else if (pressed == "right") {
			_selected++;
			ez.clearCanvas();
			_drawImage(_items[_selected].image);
			_drawCaption();
		} else if ( (ez.isBackExitOrDone(name) && !_items[_selected].advancedFunction) || ez.isBackExitOrDone(pressed) ) {
			_pick_button = pressed;
			_selected = -1;
			ez.background(_old_background);
			ez.clearScreen();
			return 0;
		} else {
			// Some other key must have been pressed. We're done here!
			ez.clearScreen();
			_pick_button = pressed;
			ez.background(_old_background);
			if (_items[_selected].simpleFunction != NULL) {
				(_items[_selected].simpleFunction)();
				ez.clearScreen();
			}
			if (_items[_selected].advancedFunction != NULL) {
				if (!(_items[_selected].advancedFunction)(this)) {
					ez.clearScreen();
					return 0;
				} else {
					ez.clearScreen();
				}
			}
			return _selected + 1; 	// We return items starting at one, but work starting at zero internally
		}
	}			
}

void ezMenu::_drawImage(const char *image) {
	m5.lcd.drawJpg((uint8_t *)image, (sizeof(image) / sizeof(image[0])), 0, ez.canvasTop() + _img_from_top, TFT_W, ez.canvasHeight() - _img_from_top);
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
			x = ez.canvasLeft() + _img_caption_hmargin;
			break;
		case TC_DATUM:
		case MC_DATUM:
		case BC_DATUM:
			x = ez.canvasLeft() + ez.canvasWidth() / 2;
			break;
		case TR_DATUM:
		case MR_DATUM:
		case BR_DATUM:
			x = ez.canvasRight() - _img_caption_hmargin;
			break;
		default:
			// unsupported datum
			return;
	}
	switch(_img_caption_location) {
		case TL_DATUM:
		case TC_DATUM:
		case TR_DATUM:
			y = ez.canvasTop() + _img_caption_vmargin;
			break;
		case ML_DATUM:
		case MC_DATUM:
		case MR_DATUM:
			y = ez.canvasTop() + ez.canvasHeight() / 2;
			break;
		case BL_DATUM:
		case BC_DATUM:
		case BR_DATUM:
			y = ez.canvasBottom() - _img_caption_vmargin;
			break;
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

void ezMenu::_Arrows() {
	uint16_t fill_color;

	int16_t top = ez.canvasTop();
	int16_t height = ez.canvasHeight();

	// Up arrow
	if (_offset > 0) {
		fill_color = MENU_ITEM_COLOR;
	} else {
		fill_color = ez.background();
	}
	m5.lcd.fillTriangle(15, top + 25, 25, top + 25, 20, top + 10, fill_color);

	// Down arrow
	if (_items.size() > _offset + _items_per_screen) {
		fill_color = MENU_ITEM_COLOR;
	} else {
		fill_color = ez.background();
	}
	m5.lcd.fillTriangle(15, top + height - 25, 25, top + height - 25, 20, top + height - 10, fill_color);		
}


M5ez ez;
