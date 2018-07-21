#ifndef _M5EZ_H_
#define _M5EZ_H_

// Uncomment to support the FACES keyboard
// #define M5EZ_WITH_FACES

// Have the autoconnect logic print debug messages on the serial port
// #define M5EZ_WIFI_DEBUG

#include <vector>			// std::vector
#include <WiFi.h>			// WiFiEvent_t, system_event_info_t
#include <M5Stack.h>		// GFXfont*

// Special fake font pointers to access the older non FreeFonts in a unified way.
// Only valid if passed to ez.setFont
// (Note that these need to be specified without the & in front, unlike the FreeFonts)
#define mono6x8				(GFXfont*) 1
#define sans16				(GFXfont*) 2
#define sans26				(GFXfont*) 4
#define numonly48			(GFXfont*) 6
#define numonly7seg48		(GFXfont*) 7
#define numonly75			(GFXfont*) 8
// The following fonts are just scaled up from previous ones (textSize 2)
// But they might still be useful.
#define mono12x16			(GFXfont*) 9
#define sans32				(GFXfont*) 10
#define sans52				(GFXfont*) 12
#define numonly96			(GFXfont*) 14
#define numonly7seg96		(GFXfont*) 15
#define numonly150			(GFXfont*) 16

// Just ignore this. It does NOT mean anything is printed transparent by default...
// It just signals to ez.print and ez.println to use the current default color if
// no color is specified. The actual default print color is defined in the theme file. 
#define PRINT_DEFAULT_COL	TFT_TRANSPARENT

#define TFT_W 				320
#define TFT_H				240

#ifndef M5EZ_THEME_LOADED
#include <themes/default.h>
#endif

class M5ez {

	public:

		M5ez();

		void yield();

		// Screen, canvas, etc...
		void clearScreen();
		void clearScreen(uint16_t color);
		void background(uint16_t color);
		uint16_t background();
		void clearCanvas();
		int16_t canvasTop();
		int16_t canvasBottom();
		int16_t canvasLeft();
		int16_t canvasRight();
		int16_t canvasWidth();
		int16_t canvasHeight();
		
		// Header
		void drawHeader(String header_txt);
		void removeHeader();

		// Buttons
		void drawButtons(String buttons);
		String getButtons();
		String waitForButtons();
		String waitForButtons(String buttons);

		// ez.msgBox
		String msgBox(String header, String msg, String buttons = "OK", const bool blocking = true, const GFXfont* font = MSG_FONT, uint16_t color = MSG_COLOR);

		// ez.textInput
		String textInput(String header = "", String defaultText = "");
		
		// FACES support
		String getFACES();
	
		// ez.print
		void print(String text, int16_t x = -1, int16_t y = -1, const GFXfont* font = NULL, uint16_t color = PRINT_DEFAULT_COL);
		void println(String text, int16_t x = -1, int16_t y = -1, const GFXfont* font = NULL, uint16_t color = PRINT_DEFAULT_COL);
		void printFont(const GFXfont* font);
		void printColor(uint16_t color);	
		void printLmargin(int16_t lmargin);
		// void printScroll(bool state);
		void printWrap(bool state);
		// void scrollCanvas(int16_t pixels);	//Not supported until we get m5.lcd.readRect(...) to work
		
		// wifi
		void _wifiSignalBars(bool now = false);
		
#ifndef M5EZ_WITHOUT_WIFI
		// ezWifiMenu
		void wifiAddNetwork(String SSID, String key);
		bool wifiDeleteNetwork(int8_t index);
		bool wifiDeleteNetwork(String ssid);
		int8_t getIndexForSSID(String ssid);
		uint8_t wifiNumNetworks();
		String wifiSSID(uint8_t index);
		String wifiKey(uint8_t index);
		bool wifiAutoconnectOn();
		void wifiAutoconnectOn(bool new_state);
		void wifiStatus();		// Old entry point for ezWifiMenu()
		void wifiReadFlash();
		void wifiWriteFlash();
#endif
			
		// Generic String object helper functions
		String rightOf(String input, String separator, bool trim = true);
		String leftOf(String input, String separator, bool trim = true);
		int16_t countStringInString(String haystack, String needle);
		int16_t chopStringIntoArray(String input, String separator, String array[], bool trim = true);
		int16_t charsFit(String input, int16_t cutoff);
		String clipString(String input, int16_t cutoff, bool dots = true);
		bool isBackExitOrDone(String str);
		
		// m5.lcd wrappers that make fonts easier
		void setFont(const GFXfont* font);
		int16_t fontHeight();
		
		
		// Color helpers
		// uint16_t to16bit(RGB color);
		// uint16_t to16bit(String htmlcolor);
		// RGB toRGB(String htmlcolor);
		// RGB toRGB(uint16_t color);

	private:
	
	
		// Screen, canvas, etc...
		uint16_t _background;
		bool _lower_button_row;
		bool _upper_button_row;   	
		bool _header_displayed;
		int16_t _canvas_t, _canvas_h;
		void _setCanvasVars();

		// buttons
	   	String _btn_a_s, _btn_a_l;
		String _btn_b_s, _btn_b_l;
		String _btn_c_s, _btn_c_l;
		String _btn_ab, _btn_bc, _btn_ac;
		bool _key_release_wait;
		void _drawButtons(String btn_a_s, String btn_a_l, String btn_b_s, String btn_b_l, String btn_c_s, String btn_c_l, String btn_ab, String btn_bc, String btn_ac);
		void _drawButton(int16_t row, String text_s, String text_l, int16_t x, int16_t w);
		void _drawButtonString(String text, int16_t x, int16_t y, uint16_t color, int16_t datum);
	
		// ez.textInput
		int16_t _text_cursor_x, _text_cursor_y, _text_cursor_h, _text_cursor_w;
		bool _text_cursor_state;
		void _drawTextInputLockString(String text);
		void _drawTextInputBox(String text);
		void _textCursor();
		void _textCursor(bool state);
		long  _text_cursor_millis;
		
		// FACES keyboard support
		uint8_t _faces_state;

		// ez.print
		const GFXfont* _print_font;
		uint16_t _print_color;
		int16_t _print_x, _print_y, _print_lmargin;
		bool _print_wrap;
		// bool _print_scroll;		//Not supported until we get m5.lcd.readRect(...) to work
		
};

class ezMenu {

	public:

		ezMenu(String hdr = "");
		~ezMenu();
		bool addItem(String nameAndCaption, void (*simpleFunction)() = NULL, bool (*advancedFunction)(ezMenu* callingMenu) = NULL);
		bool addItem(const char *image, String nameAndCaption, void (*simpleFunction)() = NULL, bool (*advancedFunction)(ezMenu* callingMenu) = NULL);
		bool deleteItem(int16_t index);
		bool deleteItem(String name);
		bool setCaption(int16_t index, String caption);
		bool setCaption(String name, String caption);
		void buttons(String bttns);
		int16_t getItemNum(String name);
		int16_t pick();
		String pickName(), pickCaption(), pickButton();
		void run();
		int16_t runOnce();
		void txtBig();
		void txtSmall();
		void txtFont(const GFXfont* font);
		void imgBackground(uint16_t color);
		void imgFromTop(int16_t offset);
		void imgCaptionFont(const GFXfont* font);	
		void imgCaptionLocation(uint8_t datum);
		void imgCaptionColor(uint16_t color);
		void imgCaptionMargins(int16_t hmargin, int16_t vmargin);
		void imgCaptionMargins(int16_t margin);
		
	private:
	
		struct MenuItem_t {
			String nameAndCaption;
			const char *image;
			void (*simpleFunction)();
			bool (*advancedFunction)(ezMenu* callingMenu);
		};
		std::vector<MenuItem_t> _items;
		int16_t _selected, _offset;
		String _header, _buttons, _pick_button;
		int16_t _per_item_h, _vmargin;
		int16_t _items_per_screen;
		uint16_t _old_background;
		void _drawImage(const char *image);
		void _drawCaption();
		const GFXfont* _font;
		int16_t _runImagesOnce();
		int16_t _runTextOnce();
		void _fixOffset();
		void _drawItems();
		void _drawItem(int16_t n, String text, bool selected);
		void _Arrows();
		int16_t _img_from_top;
		uint8_t _img_caption_location;
		uint16_t _img_caption_color;
		uint16_t _img_background;
		const GFXfont* _img_caption_font;
		int16_t _img_caption_hmargin, _img_caption_vmargin;	

};

extern M5ez ez;

#ifndef M5EZ_WITHOUT_WIFI
extern void ezWifiMenu();
namespace {
	void wifiJoin();
	WiFiEvent_t _WPS_event;
	String _WPS_pin;
	bool _WPS_new_event;
	void _WPShelper(WiFiEvent_t event, system_event_info_t info);
	long _last_wifi_signal_update;
	struct WifiNetwork_t {
		String SSID;
		String key;
	};
	std::vector<WifiNetwork_t> _networks;
	bool _wifi_autoconnect_on;
	enum WifiState_t {
		EZWIFI_NOT_INIT,
		EZWIFI_WAITING,
		EZWIFI_IDLE,
		EZWIFI_SCANNING,
		EZWIFI_CONNECTING,
		EZWIFI_AUTOCONNECT_DISABLED
	};
	WifiState_t _wifi_state;
	uint8_t _wifi_current_from_scan;
	int32_t _wifi_until;
	void _wifiLoop();
	bool _wifiConnection(ezMenu* callingMenu);
	bool _wifiOnOff(ezMenu* callingMenu);
	bool _wifiBack(ezMenu* callingMenu);
	void _wifiAskAdd();
	void _wifiManageAutoconnects();
	bool _wifiAutoconnectSelected(ezMenu* callingMenu);
}
#endif // M5EZ_WITHOUT_WIFI

#endif	//_M5EZ_H_
