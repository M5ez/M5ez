#include <M5ez.h>
#include "ScreenShotExtension.h"

ScreenShotExtension sce;

void setup() {

  ez.install("SnapShot", sce.control);
  ez.begin();
}

void loop() {
  ez.settings.menu();
}
