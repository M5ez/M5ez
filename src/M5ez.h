#ifndef _M5EZ_H_
#define _M5EZ_H_

#define M5EZ_VERSION		"2.1.2"


// Comment out the line below to disable WPS.
#define M5EZ_WPS

// Turn this off to compile without WiFi (no) OTA updates, no clock)
#define M5EZ_WIFI

// Turn this off if you don't have a battery attached
#define M5EZ_BATTERY

// Turn this off to compile without BLE (Bluetooth Low Energy)
// #define M5EZ_BLE
#ifdef M5EZ_BLE
	#define M5EZ_BLE_DEVICE_NAME "M5ez"
#endif

// Have the autoconnect logic print debug messages on the serial port
// #define M5EZ_WIFI_DEBUG

// Determines whether the backlight is settable
#define M5EZ_BACKLIGHT

// Compile in ezTime and create a settings menu for clock display
#define M5EZ_CLOCK

// FACES settings menu
#define M5EZ_FACES

#include <vector>			// std::vector
#ifdef M5EZ_WIFI
	#include <WiFi.h>			// WiFiEvent_t, system_event_info_t
#endif
#include <M5Stack.h>		// GFXfont*
#ifdef M5EZ_CLOCK
	#include <ezTime.h>			// events, on-screen clock
#endif
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

#define NO_COLOR			TFT_TRANSPARENT

#define TFT_W		320
#define TFT_H		240

struct line_t {
	int16_t position;
	String line;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   T H E M E
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ezTheme {
	public:
		static void begin();
		void add();
		static bool select(String name);
		static void menu();

		String name = "Default";								// Change this when making theme
		uint16_t background = 0xEF7D;
		uint16_t foreground = TFT_BLACK;
		uint8_t header_height = 23;
		const GFXfont* header_font = &FreeSansBold9pt7b;
		uint8_t header_hmargin = 5;
		uint8_t header_tmargin = 3;
		uint16_t header_bgcolor = TFT_BLUE;
		uint16_t header_fgcolor = TFT_WHITE;					

		const GFXfont* print_font = mono6x8;					
		uint16_t print_color = foreground;					
		
		const GFXfont* clock_font = mono12x16;

		uint16_t longpress_time = 250;							//milliseconds

		uint8_t button_height = 19;								
		const GFXfont* button_font = &FreeSans9pt7b;			
		uint8_t button_tmargin = 1;								
		uint8_t button_hmargin = 5;								
		uint8_t button_gap = 3;									
		uint8_t button_radius = 8;								
		uint16_t button_bgcolor_b = TFT_BLUE;					
		uint16_t button_bgcolor_t = TFT_PURPLE;					
		uint16_t button_fgcolor = TFT_WHITE;					
		uint16_t button_longcolor = TFT_CYAN;					

		uint8_t input_top = 50;									// pixels below ez.canvas.top()
		uint8_t input_hmargin = 10;								// The distance between text box and edge of screen
		uint8_t input_vmargin = 10;								// Vertical margin _inside_ the text box
		const GFXfont* input_font = &FreeMonoBold12pt7b;		
		const GFXfont* input_keylock_font = &FreeSansBold9pt7b;
		uint16_t input_bgcolor = TFT_BLACK;						
		uint16_t input_fgcolor = TFT_GREEN;						
		uint16_t input_cursor_blink = 500;						// milliseconds
		uint8_t input_faces_btns = 18;							

		const GFXfont* tb_font = &FreeSans9pt7b;				
		uint16_t tb_color = foreground;							
		uint8_t tb_hmargin = 5;

		uint8_t menu_lmargin = 20;								
		uint8_t menu_rmargin = 10;
		uint8_t menu_arrows_lmargin = 5;							
		uint16_t menu_item_color = foreground;					
		uint16_t menu_sel_bgcolor = foreground;					
		uint16_t menu_sel_fgcolor = background;					
		const GFXfont* menu_big_font = &FreeSans12pt7b;			
		const GFXfont* menu_small_font = &FreeSans9pt7b;		
		uint8_t menu_item_hmargin = 10;							
		uint8_t menu_item_radius = 8;

		const GFXfont* msg_font = &FreeSans12pt7b;				
		uint16_t msg_color = foreground;						
		uint8_t msg_hmargin = 20;								

		uint8_t progressbar_line_width = 4;						
		uint8_t progressbar_width = 25;							
		uint16_t progressbar_color = foreground;				

		uint16_t signal_interval = 2000;						
		uint8_t signal_bar_width = 4;							
		uint8_t signal_bar_gap = 2;

		uint8_t battery_bar_width = 26;
		uint8_t battery_bar_gap = 2;
		uint32_t battery_0_fgcolor = TFT_RED; 
		uint32_t battery_25_fgcolor = TFT_ORANGE;
		uint32_t battery_50_fgcolor = TFT_YELLOW;
		uint32_t battery_75_fgcolor = TFT_GREENYELLOW;
		uint32_t battery_100_fgcolor = TFT_GREEN;
	//						
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   S C R E E N
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ezScreen {
	public:
		static void begin();
		static void clear();
		static void clear(uint16_t color);
		static uint16_t background();
	private:
		static uint16_t _background;
	//
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   H E A D E R
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct header_widget_t {
	String name;
	bool leftover;		// widget gets all leftover width. Used by title, only one widget can have this property
	uint16_t x;
	uint16_t w;
	void (*function)(uint16_t x, uint16_t w);
};	
#define LEFTMOST		0
#define RIGHTMOST		255

class ezHeader {
	private:
		static std::vector<header_widget_t> _widgets;
		static bool _shown;
		static String _title;
		static void _drawTitle(uint16_t x, uint16_t w);
		static void _recalculate();
	public:
		static void begin();
		static void show(String t = "");
		static bool shown();
		static void clear(bool wipe = true);
		static void insert(uint8_t position, String name, uint16_t width, void (*function)(uint16_t x, uint16_t w), bool leftover = false);
		static void remove(String name);
		static uint8_t position(String name);
		static void draw(String name = "");
		static String title();
		static void title(String title);
	//
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   C A N V A S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct print_t {
	const GFXfont* font;
	uint16_t color;
	uint16_t x;
	int16_t y;
	String text;
};	

class ezCanvas : public Print {
	friend class ezHeader;
	friend class ezButtons;
	public:
		static void begin();
		static void reset();
		static void clear();
		static uint8_t top();
		static uint8_t bottom();
		static uint16_t left();
		static uint16_t right();
		static uint8_t height();
		static uint16_t width();
		static bool scroll();
		static void scroll(bool s);
		static bool wrap();
		static void wrap(bool w);		
		static uint16_t lmargin();
		static void lmargin(uint16_t newmargin);
		static void font(const GFXfont* font);
		static const GFXfont* font();
		static void color(uint16_t color);
		static uint16_t color();
		static void pos(uint16_t x, uint8_t y);
		static uint16_t x();
		static void x(uint16_t newx);
		static uint8_t y();
		static void y(uint8_t newy);
		virtual size_t write(uint8_t c);						// These three are used to inherint print and println from Print class
		virtual size_t write(const char *str);
		virtual size_t write(const uint8_t *buffer, size_t size);
		static uint16_t loop();
	private:
		static std::vector<print_t> _printed;
		static uint32_t _next_scroll;
		static void top(uint8_t newtop);
		static void bottom(uint8_t newbottom);
		static uint8_t _y, _top, _bottom;
		static uint16_t _x, _left, _right, _lmargin;
		static bool _wrap, _scroll;
		static const GFXfont* _font;
		static uint16_t _color;
		static void _print(String text);
		static void _putString(String text);
	//
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   B U T T O N S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ezButtons {
	public:
		static void begin();
		static void show(String buttons);
		static void clear(bool wipe = true);
		static void releaseWait();
		static String poll();
		static String wait(String Buttons = ""); 
	private:
		static String _btn_a_s, _btn_a_l;
		static String _btn_b_s, _btn_b_l;
		static String _btn_c_s, _btn_c_l;
		static String _btn_ab, _btn_bc, _btn_ac;
		static bool _key_release_wait;
		static bool _lower_button_row, _upper_button_row;
		static void _drawButtons(String btn_a_s, String btn_a_l, String btn_b_s, String btn_b_l, String btn_c_s, String btn_c_l, String btn_ab, String btn_bc, String btn_ac);
		static void _drawButton(int16_t row, String text_s, String text_l, int16_t x, int16_t w);
		static void _drawButtonString(String text, int16_t x, int16_t y, uint16_t color, int16_t datum);
	//
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   M E N U
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ezMenu {
	public:
		ezMenu(String hdr = "");
		bool addItem(String nameAndCaption, void (*simpleFunction)() = NULL, bool (*advancedFunction)(ezMenu* callingMenu) = NULL, void (*drawFunction)(ezMenu* callingMenu, int16_t x, int16_t y, int16_t w, int16_t h) = NULL);
		bool addItem(const char *image, String nameAndCaption, void (*simpleFunction)() = NULL, bool (*advancedFunction)(ezMenu* callingMenu) = NULL, void (*drawFunction)(ezMenu* callingMenu, int16_t x, int16_t y, int16_t w, int16_t h) = NULL);
		bool addItem(fs::FS &fs, String path, String nameAndCaption, void (*simpleFunction)() = NULL, bool (*advancedFunction)(ezMenu* callingMenu) = NULL, void (*drawFunction)(ezMenu* callingMenu, int16_t x, int16_t y, int16_t w, int16_t h) = NULL);
		bool deleteItem(int16_t index);
		bool deleteItem(String name);
		bool setCaption(int16_t index, String caption);
		bool setCaption(String name, String caption);
		void setSortFunction(bool (*sortFunction)(const char* s1, const char* s2));
		void buttons(String bttns);
		void upOnFirst(String nameAndCaption);
		void leftOnFirst(String nameAndCaption);
		void downOnLast(String nameAndCaption);
		void rightOnLast(String nameAndCaption);
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

		static bool sort_asc_name_cs (const char* s1, const char* s2);
		static bool sort_asc_name_ci (const char* s1, const char* s2);
		static bool sort_dsc_name_cs (const char* s1, const char* s2);
		static bool sort_dsc_name_ci (const char* s1, const char* s2);
		static bool sort_asc_caption_cs (const char* s1, const char* s2);
		static bool sort_asc_caption_ci (const char* s1, const char* s2);
		static bool sort_dsc_caption_cs (const char* s1, const char* s2);
		static bool sort_dsc_caption_ci (const char* s1, const char* s2);

	private:
		struct MenuItem_t {
			String nameAndCaption;
			const char *image;
			fs::FS *fs;
			String path;
			void (*simpleFunction)();
			bool (*advancedFunction)(ezMenu* callingMenu);
			void (*drawFunction)(ezMenu* callingMenu, int16_t x, int16_t y, int16_t w, int16_t h);
		};
		std::vector<MenuItem_t> _items;
		int16_t _selected, _offset;
		bool _redraw;
		String _header, _buttons, _pick_button;
		String _up_on_first, _down_on_last;
		int16_t _per_item_h, _vmargin;
		int16_t _items_per_screen;
		uint16_t _old_background;
		void _drawImage(MenuItem_t &item);
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
		bool (*_sortFunction)(const char* s1, const char* s2);
		void _sortItems();
		bool _sortWrapper(MenuItem_t& item1, MenuItem_t& item2);
	//
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   P R O G R E S S B A R
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ezProgressBar {
	public:
		ezProgressBar(String header = "", String msg = "", String buttons = "", const GFXfont* font = NULL, uint16_t color = NO_COLOR, uint16_t bar_color = NO_COLOR);
		void value(float val);
	private:
		int16_t _bar_y;
		uint16_t _bar_color;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   S E T T I N G S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ezSettings {
	public:
		static void begin();
		static void menu();
		static void defaults();
		static ezMenu menuObj;
	//
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   B A C K L I G H T
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_BACKLIGHT
	#define NEVER		0
	#define USER_SET	255
	class ezBacklight {
		public:
			static void begin();
			static void menu();
			static void inactivity(uint8_t half_minutes);
			static void activity();
			static uint16_t loop();
		private:
			static uint8_t _brightness;
			static uint8_t _inactivity;
			static uint32_t _last_activity;
			static bool _backlight_off;
		//
	};
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   C L O C K
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_CLOCK
	class ezClock {
		public:
			static Timezone tz;
			static void begin();
			static void restart();
			static void menu();
			static uint16_t loop();
			static void clear();
			static void draw(uint16_t x, uint16_t w);
			static bool waitForSync(const uint16_t timeout = 0);
		private:
			static void _writePrefs();
			static bool _on;
			static String _timezone;
			static bool _clock12;
			static bool _am_pm;
			static String _datetime;
			static bool _starting;
		//
	};
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   F A C E S
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_FACES
	class ezFACES {
		public:
			static void begin();
			static void menu();
			static String poll();
			static bool on();
		private:
			static bool _on;
		//
	};
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   W I F I
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_WIFI

	enum WifiState_t {
		EZWIFI_NOT_INIT,
		EZWIFI_WAITING,
		EZWIFI_IDLE,
		EZWIFI_SCANNING,
		EZWIFI_CONNECTING,
		EZWIFI_AUTOCONNECT_DISABLED
	};

	struct WifiNetwork_t {
		String SSID;
		String key;
	};

	class ezWifi {
		public:
			static std::vector<WifiNetwork_t> networks;
			static bool autoConnect;
			static void begin();
			static void add(String ssid, String key);
			static bool remove(int8_t index);
			static bool remove(String ssid);
			static int8_t indexForSSID(String ssid);
			static void readFlash();
			static void writeFlash();
			static void menu();
			static uint16_t loop();
			static bool update(String url, const char* root_cert, ezProgressBar* pb = NULL);
			static String updateError();
		private:
			static WifiState_t _state;
			static uint8_t _current_from_scan;
			static uint32_t _wait_until, _widget_time;
			static void _drawWidget(uint16_t x, uint16_t w);
			static bool _onOff(ezMenu* callingMenu);
			static void _manageAutoconnects();
			static bool _autoconnectSelected(ezMenu* callingMenu);
			static void _askAdd();
			static bool _connection(ezMenu* callingMenu);
			static void _update_progress(int done, int total); 
			static String _update_err2str(uint8_t _error);
			static ezProgressBar* _update_progressbar;
			static String _update_error;
			#ifdef M5EZ_WPS
				static void _WPShelper(WiFiEvent_t event, system_event_info_t info);
				static WiFiEvent_t _WPS_event;
				static String _WPS_pin;
				static bool _WPS_new_event;
			#endif
		//
	};

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   B L E
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_BLE

	class ezBLE {
		public:
			static void begin();
			static void readFlash();
			static void writeFlash();
			static void menu();
			static void disconnect();
			static class BLEClient* getClient(uint16_t index);
			static uint16_t getClientCount();
		private:
			static const std::vector<std::pair<uint16_t, String>> _gattUuids;
			static bool _on;
			static bool _initialized;
			static std::vector<class BLEClient*> _clients;
			static bool _scan(ezMenu* callingMenu);
			static void _connect(class BLEAdvertisedDevice& device);
			static bool _listClients(ezMenu* callingMenu);
			static bool _showClient(class BLEClient* client);
			static void _cleanup();
			static void _refresh();
			friend class M5ezClientCallback;
	};

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   B A T T E R Y
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef M5EZ_BATTERY

	class ezBattery {
		public:
			static void begin();
			static void readFlash();
			static void writeFlash();
			static void menu();
			static uint16_t loop();
			static uint8_t getTransformedBatteryLevel();
			static uint32_t getBatteryBarColor(uint8_t batteryLevel);
		private:
			static bool _on;
			static void _refresh();
			static void _drawWidget(uint16_t x, uint16_t w);
	};

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   E Z
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct event_t {
	uint16_t (*function)();
	uint32_t when;
};
class M5ez {

	friend class ezProgressBar;
	friend class ezHeader;		// TMP?
	friend class ezMenu;		// For _redraw

	public:
		static std::vector<ezTheme> themes;
		static ezTheme* theme;
		static ezScreen screen;
		static constexpr ezScreen& s = screen;
		static ezHeader header;
		static constexpr ezHeader& h = header;
		static ezCanvas canvas;
		static constexpr ezCanvas& c = canvas;
		static ezButtons buttons;
		static constexpr ezButtons& b = buttons;
		static ezSettings settings;
		#ifdef M5EZ_WIFI
			static ezWifi wifi;
			static constexpr ezWifi& w = wifi;
		#endif
		#ifdef M5EZ_BLE
			static ezBLE ble;
		#endif
		#ifdef M5EZ_BATTERY
			static ezBattery battery;
		#endif
		#ifdef M5EZ_BACKLIGHT
			static ezBacklight backlight;
		#endif
		#ifdef M5EZ_CLOCK
			static ezClock clock;
		#endif
		#ifdef M5EZ_FACES
			static ezFACES faces;
		#endif	
		
		static void begin();

		static void yield();
		
		static void addEvent(uint16_t (*function)(), uint32_t when = 1);
		static void removeEvent(uint16_t (*function)());
		static void redraw();

		// ez.msgBox
		static String msgBox(String header, String msg, String buttons = "OK", const bool blocking = true, const GFXfont* font = NULL, uint16_t color = NO_COLOR);

		// ez.textInput
		static String textInput(String header = "", String defaultText = "");
		
		//ez.textBox
		static String textBox(String header = "", String text = "", bool readonly = false, String buttons = "up#Done#down", const GFXfont* font = NULL, uint16_t color = NO_COLOR);
			
		// Generic String object helper functions
		static String rightOf(String input, String separator, bool trim = true);
		static String leftOf(String input, String separator, bool trim = true);
		static int16_t countStringInString(String haystack, String needle);
		static int16_t chopString(String input, String separator, std::vector<String>& chops, bool trim = true);
		static int16_t charsFit(String input, int16_t cutoff);
		static String clipString(String input, int16_t cutoff, bool dots = true);
		static bool isBackExitOrDone(String str);
		
		// m5.lcd wrappers that make fonts easier
		static void setFont(const GFXfont* font);
		static int16_t fontHeight();
		
		static String version();

	private:
		static std::vector<event_t> _events;
		static bool _redraw;


		// ez.textInput
		static int16_t _text_cursor_x, _text_cursor_y, _text_cursor_h, _text_cursor_w;
		static bool _text_cursor_state;
		static void _drawTextInputLockString(String text);
		static void _drawTextInputBox(String text);
		static void _textCursor();
		static void _textCursor(bool state);
		static long  _text_cursor_millis;

		// ez.textBox
		static void _wrapLines(String text, uint16_t width, std::vector<line_t>& lines);
		static void _fitLines(String text, uint16_t max_width, uint16_t min_width, std::vector<line_t>& lines);
	//
};

extern M5ez ez;


#endif	//_M5EZ_H_
