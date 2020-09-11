#include <M5ez.h>
#include "ScreenShotFeature.h"

ScreenShotFeature ss;

void setup() {
	ez.remove("ezBLE");
	ez.remove("ezFACES");
	ez.begin();
	ez.add("ScreenShot", ss.entry);
}

void loop() {
	ez.settings.menu();
}
