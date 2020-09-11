#include <M5ez.h>
#include "feature.h"

// To control what features are installed when M5ez starts up, remove those you don't want by name.
// Available names are (currently): ezWifi, ezFACES, ezBacklight, ezClock, ezBattery, ezBLE.
// Add your own features by:
//  Copying and modifying feature.h and feature.cpp.
//  Calling ez.add() with the feature's name and the address for its static bool entry(uint8_t command, void* data)

void setup() {
  ez.remove("ezBLE");
  ez.remove("ezBacklight");
  ez.remove("ezFACES");
  ez.add(FEATURE_NAME, Feature::entry);
  ez.begin();
}

void loop() {
  ez.settings.menu();
}
