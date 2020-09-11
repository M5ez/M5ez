#include <Preferences.h>
#include "feature.h"

// For settings that you'll save to Preferences, choose a name that describes the feature followed
// by an underscore, and a description of the data. Identifier MUST BE 15 characters or less.
// You will need a name for each piece of data saved. Preferences does not save data structures.
#define FEATURE_PREFERENCE_DATUM	"feature_datum"


// Static class member initialization
const String Feature::_name = FEATURE_NAME;
bool Feature::_datum = false;


// entry() is the main control center for the feature.
// You must respond to FEATURE_MSG_PING and FEATURE_MSG_START.
// Handling FEATURE_MSG_STOP is recommended, if that makes any sense for your extension.
// (Note: the system does not currently call FEATURE_MSG_STOP, but may in the future.)
// PAUSE, RESUME, and QUERY_ENABLED are optional. If you need them, please use these command IDs.
// If you need custom command ids of your own, use any numbers above 100.
//
bool Feature::entry(uint8_t command, void* data) {
	switch(command) {
		case FEATURE_MSG_PING:
			return true;		// Always return true, even if disabled.
		case FEATURE_MSG_START:
			// If you have a lot to do on startup, add a private function
			_readPrefs();		// Get stored settings and initialize state
			ez.addEvent(loop);	// Ensure that this extension is called periodically
			ez.settings.menuObj.addItem(_name, menu);	// Make it so we show up in settings menu.
			return true;
		case FEATURE_MSG_STOP:
			return false;	// Be honest. This example has nothing to stop.
		case FEATURE_MSG_QUERY_ENABLED:
			return true;	// There is no way to disable this simple feature.
	}
	return false;
}


// This function is called by ez.settings.menu(), and displays a menu intended for viewing and
// modifying the preferences for this feature.
// If you have no settings or state for the user to modify, don't call ez.settings.menuObj.addItem(_name, menu)
// in entry() and delete this function.
// Since this is a sub-menu, it's advantageous to include .buttons("up # back # select ## down #")
// For consistency, you might want to include .txtSmall()
//
void Feature::menu() {
	bool original_datum = _datum;
	String disp = "Datum";
	disp += _datum ? "\tTrue" : "\tFalse";
	ezMenu menu(_name);
	menu.txtSmall();
	menu.buttons("up # back # select ## down #");
	menu.addItem(disp);
	while(menu.runOnce()) {
		if(1 == menu.pick()) {
			_datum = !_datum;
			disp = "Datum";
			disp += _datum ? "\tTrue" : "\tFalse";
			menu.setCaption(1, disp);
		}
	}
	// If the data has changed, write it to Preferences
	if(_datum != original_datum) {
		_writePrefs();
	}
}


// This template doesn't have much work to do, but real code often does.
// If you have nothing to do periodically, don't call ez.addEvent(loop) in entry()
// and delete this function. Otherwise, do your work quickly here and then
// return the number of milliseconds you'd like to pass before you're called again.
// You can access the currently displayed menu (if any) via "ezMenu* ez.getCurrentMenu()"
//
uint16_t Feature::loop() {
	// Serail.println("Feature::loop(), returning 1000.");
	return 1000;
}


// Save our preferences to Non Volatile Storage (NVS)
// Note: if you 'change your mind' about a data type, you must call prefs.remove(name) before
// you can save/read a different type. Calling with a different type is simply an error.
//
void Feature::_readPrefs() {
	Preferences prefs;
	prefs.begin("M5ez", true);	// Open prefs read-only if no changes are intended
	_datum = prefs.getBool(FEATURE_PREFERENCE_DATUM, 0);	// There are different 'getters' for each data type
	// Normally there would be a number of preferences to store here
	prefs.end();				// Close preferences right away; it's not intended to be left open.
}


// Read our preferences from NVS
//
void Feature::_writePrefs() {
	Preferences prefs;
	prefs.begin("M5ez", false);	// Open prefs read-write when storing values. Be sure to close.
	prefs.putBool(FEATURE_PREFERENCE_DATUM, _datum);	// Select the correct putter for your data
	// Normally there would be a number of preferences to load here
	prefs.end();
}
