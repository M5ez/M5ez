#include <M5ez.h>
#include "ScreenShotExtension.h"

ScreenShotExtension sce;

void setup() {
  ez.install(sce.begin);
  ez.begin();
}

void loop() {
  ez.settings.menu();
}
