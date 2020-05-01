// For more information about the M5Stack file formats, please refer to the following blog post: 
// https://schrenk.hu/2020/04/29/M5StackImageFormats/
//
#include <Arduino.h>
#include <ezTime.h>
#include <M5ez.h>

// Image data in FLASH memory
#include "jpgs.h"
#include "jpgsdark.h"
#include "bmps.h"
#include "xbmps.h"

/////////////////////
// Utility methods //
/////////////////////

void aboutM5ez() 
{  
  ez.msgBox("About M5ez", "M5ez was written by | Rop Gonggrijp | | https://github.com/ropg/M5ez");
}

void powerOff() { 
  m5.Power.powerOFF(); 
}

///////////////
// MAIN MENU //
///////////////

ezMenu initMainMenu()
{
  ezMenu mainMenu;

  //Center the 96x96 px image on the screen
  mainMenu.imgFromTop(72);
  mainMenu.imgFromLeft(112);

  if (ez.theme->name == "Default") {  
    mainMenu.imgBackground(ez.theme->background);
    mainMenu.imgCaptionColor(ez.theme->header_bgcolor);
  } else if (ez.theme->name == "Dark") {
    mainMenu.imgBackground(ez.theme->background);
    mainMenu.imgCaptionColor(ez.theme->header_fgcolor);
  }

  ///////////
  // FLASH //
  ///////////

  if (ez.theme->name == "Default") {  
    //JPG files from FLASH - Default theme
    mainMenu.addItem(settings_jpg, "Settings", ez.settings.menu);
    mainMenu.addItem(about_jpg, "About", aboutM5ez);
    mainMenu.addItem(power_jpg, "Power Off", powerOff);
  } else if (ez.theme->name == "Dark") {
    //JPG files from FLASH - Dark theme
    mainMenu.addItem(settings_jpg_dark, "Settings", ez.settings.menu);
    mainMenu.addItem(about_jpg_dark, "About", aboutM5ez);
    mainMenu.addItem(power_jpg_dark, "Power Off", powerOff);
  }

  //BMP files from FLASH - Black background is transparent
  mainMenu.addBmpImageItem(settings_bmp, "BMP Settings", settings_bmp_width, settings_bmp_height, ez.settings.menu);  
  mainMenu.addBmpImageItem(about_bmp, "BMP About", about_bmp_width, about_bmp_height, aboutM5ez);  
  mainMenu.addBmpImageItem(power_bmp, "BMP Power Off", power_bmp_width, power_bmp_height, powerOff);
  
  //XBMP files in blue foreground and black background
  mainMenu.addXBmpImageItem(settings_xbmp, "XBMP Settings", settings_xbmp_width, settings_xbmp_height, TFT_BLUE, TFT_BLACK, ez.settings.menu);  
  mainMenu.addXBmpImageItem(about_xbmp, "XBMP About", about_xbmp_width, about_xbmp_height, TFT_BLUE, TFT_BLACK, aboutM5ez);  
  mainMenu.addXBmpImageItem(power_xbmp, "XBMP Power Off", power_xbmp_width, power_xbmp_height, TFT_BLUE, TFT_BLACK, powerOff);

  /////////////
  // SD card //
  /////////////

  //JPG files from SD card - Default theme
  if (ez.theme->name == "Default") {  
    mainMenu.addItem(SD, "/M5images/SettingsDefault.jpg", "SD Settings", ez.settings.menu);
    mainMenu.addItem(SD, "/M5images/AboutDefault.jpg", "SD About", aboutM5ez);
    mainMenu.addItem(SD, "/M5images/PowerDefault.jpg", "SD Power Off", powerOff);
  } else if (ez.theme->name == "Dark") {
    //JPG files from SD card - Dark theme
    mainMenu.addItem(SD, "/M5images/SettingsDark.jpg", "SD Settings", ez.settings.menu);
    mainMenu.addItem(SD, "/M5images/AboutDark.jpg", "SD About", aboutM5ez);
    mainMenu.addItem(SD, "/M5images/PowerDark.jpg", "SD Power Off", powerOff);
  }

  //Raw bmp files from SD card - only 24 bit (True Color) bmp are supported without transparency 
  //Note: BMP file format does not support transparency
  //Converter: https://online-converting.com/image/convert2bmp/
  mainMenu.addBmpImageItem(SD, "/M5images/Settings.bmp", "SD BMP Settings", ez.settings.menu);
  mainMenu.addBmpImageItem(SD, "/M5images/About.bmp", "SD BMP About", aboutM5ez);
  mainMenu.addBmpImageItem(SD, "/M5images/Power.bmp", "SD BMPPower Off", powerOff);
  
  //PNG files from SD card - its rendering is quite SLOW!!!
  mainMenu.addPngImageItem(SD, "/M5images/Settings.png", "SD PNG Settings", ez.settings.menu);
  mainMenu.addPngImageItem(SD, "/M5images/About.png", "SD PNG About", aboutM5ez);
  mainMenu.addPngImageItem(SD, "/M5images/Power.png", "SD PNG Power Off", powerOff);
  
  return mainMenu;
}

///////////////////////
// Lifecycle methods //
///////////////////////

void setup() {
  #include <themes/default.h>
  #include <themes/dark.h>
  
  ez.begin();
}

void loop() {
  initMainMenu().runOnce();
}
