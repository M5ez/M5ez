#include <M5ez.h>
#include "ScreenShotExtension.h"

ScreenShotExtension sce;

void setup() {
	ez.uninstall("ezBLE");
	ez.uninstall("ezFACES");
	ez.begin();
	ez.install("SnapShot", sce.control);
}

void loop() {
	ez.settings.menu();
}
