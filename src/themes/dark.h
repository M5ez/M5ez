#ifndef M5EZ_THEME_LOADED
#define M5EZ_THEME_LOADED

#define SCREEN_BGCOLOR		TFT_BLACK
#define THEME_FGCOLOR		TFT_WHITE

#define HEADER_HEIGHT		23
#define HEADER_FONT			&FreeSansBold9pt7b
#define HEADER_HMARGIN		5
#define HEADER_TMARGIN		3
#define HEADER_BGCOLOR		TFT_DARKGREY
#define HEADER_FGCOLOR		TFT_WHITE

#define EP_FONT				mono12x16
#define	EP_COLOR			THEME_FGCOLOR

#define LONGPRESS_TIME		250		// milliseconds

#define BUTTON_HEIGHT		19
#define BUTTON_FONT			&FreeSans9pt7b
#define BUTTON_TMARGIN		1
#define BUTTON_HMARGIN		5
#define BUTTON_GAP			3
#define BUTTON_RADIUS		8
#define BUTTON_BGCOLOR_B	TFT_BLUE
#define BUTTON_BGCOLOR_T	TFT_PURPLE
#define BUTTON_FGCOLOR		TFT_WHITE
#define BUTTON_LONGCOLOR 	TFT_CYAN

#define INPUT_TOP			50		// pixels below _canvas_t
#define INPUT_HMARGIN		10		// The distance between text box and edge of screen
#define INPUT_VMARGIN		10		// Vertical margin _inside_ the text box
#define INPUT_FONT			&FreeMonoBold12pt7b
#define INPUT_KEYLOCK_FONT	&FreeSansBold9pt7b
#define INPUT_BGCOLOR		TFT_DARKGREY
#define INPUT_FGCOLOR		TFT_GREEN
#define INPUT_CURSOR_BLINK	500		// milliseconds
#define INPUT_FACES_BTNS	18

#define TB_FONT				&FreeSans9pt7b
#define TB_COLOR			THEME_FGCOLOR
#define TB_HMARGIN			5

#define MENU_LMARGIN			40
#define MENU_RMARGIN			20
#define MENU_ITEM_COLOR			THEME_FGCOLOR
#define MENU_SEL_BGCOLOR		THEME_FGCOLOR
#define MENU_SEL_FGCOLOR		SCREEN_BGCOLOR
#define MENU_BIG_FONT			&FreeSans12pt7b
#define MENU_SMALL_FONT			&FreeSans9pt7b
#define MENU_ITEM_HMARGIN		20
#define MENU_ITEM_RADIUS		10

#define MSG_FONT			&FreeSans12pt7b
#define MSG_COLOR			THEME_FGCOLOR
#define MSG_HMARGIN			20

#define PROGRESSBAR_LINE_WIDTH	4
#define PROGRESSBAR_WIDTH		25
#define PROGRESSBAR_COLOR		THEME_FGCOLOR

#define WDGT_CAPTION_BIG		&FreeSansBold12pt7b
#define WDGT_CAPTION_SMALL		&FreeSansBold9pt7b

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

// This defines how many bars show at what signal percentage.
// Keep first bar at "anything over 0" to make sure you have connection-indication
#define SIGNAL_BARS			{ 0, 20, 40, 70 }
#define SIGNAL_INTERVAL		2000

// If you set 10 bars and a much shorter interval you get more of a wifi VU-meter 
// #define SIGNAL_BARS			{ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 }
// #define SIGNAL_INTERVAL		200


#endif		// #ifndef M5EZ_THEME_LOADED