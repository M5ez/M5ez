#include <M5ez.h>
#include "ScreenShotExtension.h"

ScreenShotExtension sce;

void setup() {
	ez.remove("ezBLE");
	ez.remove("ezFACES");
	ez.begin();
	ez.add("SnapShot", sce.entry);
}

void loop() {
	ez.settings.menu();
}
