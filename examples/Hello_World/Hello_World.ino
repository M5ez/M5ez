#include <M5ez.h>

void setup() {
	ez.begin();
}

void loop() {
	ez.msgBox("M5ez minimal program", "Hello World !", "Settings");
	ez.settings.menu();
}
