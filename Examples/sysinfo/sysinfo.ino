/* This example shows how to use M5ez to do simple display things. 
 * The "#ifndef MAIN_DECLARED" part is there so that it can also 
 * simply be added to the directory of another sketch. As long as
 * it contains "#define MAIN_DECLARED true", the sketches do not 
 * conflict and the other program can simply call sysInfo() if it 
 * wants to display system information.
 * 
 * The MAIN_DECLARED is used further down to determine whether
 * the app has an exit button. After all: if it is running stand-
 * alone there is nothing to exit to.
*/

#ifndef MAIN_DECLARED
#define MAIN_DECLARED false

#include <M5Stack.h>
#include <M5ez.h>

void setup() {
  m5.begin();
  sysInfo();
}

void loop() {

}

#endif  // #ifndef MAIN_DECLARED



void sysInfo() {
  sysInfoPage1();
  while(true) {
    String btn = ez.getButtons();
    if (btn == "up") sysInfoPage1();
    if (btn == "down") sysInfoPage2();
    if (btn == "Exit") break;
  }
}

#include <SPIFFS.h>

void sysInfoPage1() {
  const byte tab = 120;
  ez.clearScreen();
  ez.drawHeader("System Information   (1/2)");
  ez.drawButtons("#" + String(MAIN_DECLARED ? "Exit" : "") + "#down");
  ez.printFont(&FreeSans9pt7b);
  ez.printLmargin(10);
  ez.println("");
  ez.print("CPU freq:");  ez.println(String(ESP.getCpuFreqMHz()) + " MHz", tab);
  ez.print("CPU cores:");  ez.println("2", tab);    //   :)
  ez.print("Chip rev.:");  ez.println(String(ESP.getChipRevision()), tab);
  ez.print("Flash speed:");  ez.println(String(ESP.getFlashChipSpeed() / 1000000) + " MHz", tab);
  ez.print("Flash size:");  ez.println(String(ESP.getFlashChipSize() / 1000000) + " MB", tab);
  ez.print("ESP SDK v.:");  ez.println(String(ESP.getSdkVersion()), tab);
}

void sysInfoPage2() {
  const String SD_Type[5] = {"NONE", "MMC", "SD", "SDHC", "UNKNOWN"};
  const byte tab = 140;
  ez.clearScreen();
  ez.drawHeader("System Information   (2/2)");
  ez.drawButtons("up#" + String(MAIN_DECLARED ? "Exit" : "") + "#");
  ez.printFont(&FreeSans9pt7b);
  ez.printLmargin(10);
  ez.println("");
  ez.print("Free RAM:");  ez.println(String((long)ESP.getFreeHeap()) + " bytes", tab);
  ez.print("Min. free seen:");  ez.println(String((long)esp_get_minimum_free_heap_size()) + " bytes", tab);
  const int sd_type = SD.cardType();
  SPIFFS.begin();
  ez.print("SPIFFS size:");  ez.println(String((long)SPIFFS.totalBytes()) + " bytes", tab);
  ez.print("SPIFFS used:");  ez.println(String((long)SPIFFS.usedBytes()) + " bytes", tab);
  ez.print("SD type:");  ez.println(SD_Type[sd_type], tab);
  if (sd_type != 0) {
    ez.print("SD size:");  ez.println(String((long)SD.cardSize()  / 1000000) + " MB", tab);
    ez.print("SD used:");  ez.println(String((long)SD.usedBytes()  / 1000000) + " MB", tab);
  }
}
