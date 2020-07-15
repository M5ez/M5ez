#include <M5ez.h>

// A sorting method takes the form:
// bool func(const char* s1, const char* s2) { return s1 comparedTo s2; }
// Built-in sorting methos include:
// For sorting by Names as quickly as possible
// ezMenu::sort_asc_name_cs		Sort ascending by menu name, case sensitive
// ezMenu::sort_asc_name_ci		Sort ascending by menu name, case insensitive
// ezMenu::sort_dsc_name_cs		Sort descending by menu name, case sensitive
// ezMenu::sort_dsc_name_ci		Sort descending by menu name, case insensitive

// For sorting by Caption if there is one, falling back to sorting by Name if no Caption is provided (all purpose)
// ezMenu::sort_asc_caption_cs		Sort ascending by menu caption or name if no caption, case sensitive
// ezMenu::sort_asc_caption_ci		Sort ascending by menu caption or name if no caption, incase sensitive
// ezMenu::sort_dsc_caption_cs		Sort descending by menu caption or name if no caption, case sensitive
// ezMenu::sort_dsc_caption_ci		Sort descending by menu caption or name if no caption, case insensitive


void setup() {
	ez.begin();
}


void loop() {
	ezMenu menu("Menu Sorting Demo");
	menu.txtSmall();
	// You can simply call setSortFunction once here, and the menu will always stay sorted.
	// menu.setSortFunction(yourSortingFunction);
	menu.buttons("asc # Asc # dsc # Dsc # Unsorted #");
	menu.addItem("Just a few");
	menu.addItem("words of");
	menu.addItem("random text in");
	menu.addItem("No Particular Order.");
	menu.addItem("Press buttons");
	menu.addItem("to re-sort");
	menu.addItem("NAME IS IGNORED! Value is sorted. | as you like.");

	// Or, you can call setSortFunction any time to re-sort the menu.
	// Once set, additional insertions are made in sorted order as well.
	// No need to call setSortFunction again unless you want to change to a different order.
	while(true) {
		menu.runOnce();
		String result = menu.pickButton();
		if     (result == "asc")      menu.setSortFunction(ezMenu::sort_asc_caption_ci);
		else if(result == "Asc")      menu.setSortFunction(ezMenu::sort_asc_caption_cs);
		else if(result == "dsc")      menu.setSortFunction(ezMenu::sort_dsc_caption_ci);
		else if(result == "Dsc")      menu.setSortFunction(ezMenu::sort_dsc_caption_cs);
		else if(result == "Unsorted") menu.setSortFunction(nullptr);
	}
}
