#include <M5ez.h>

#include <ezTime.h>

#include "images.h"

#define MAIN_DECLARED

//static lgfx::Panel_ST7735S panel; //For M5StickC
//static lgfx::Panel_ST7789 panel;  //For M5StickC+

void setup() {
  #include <themes/default.h>
  #include <themes/dark.h>
  ezt::setDebug(INFO);
  ez.begin();
//  // Set the size of the pixels that can be displayed on the LCD panel.
//  panel.panel_width  = panel.memory_width  = TFT_W;
//  panel.panel_height = panel.memory_height = TFT_H;
//  // Set the default rotation number.
//  panel.rotation = 0;
//  // Set the number of rotation offset number.
//  panel.offset_rotation = 0;
//  // After setting up, you can pass the panel pointer to the lcd.setPanel function.
//  m5.lcd.setPanel(&panel);
//
//  //Does not work either
//  m5.lcd.setRotation(1);  //Guru Meditation Error
}

void loop() {
  ezMenu mainmenu("Welcome to M5ez");
  mainmenu.txtSmall();
  mainmenu.addItem("Flexible text menus", mainmenu_menus);
  mainmenu.addItem("Event updated menus", NULL, mainmenu_settings);
  mainmenu.addItem("Image menus", mainmenu_image);
  mainmenu.addItem("Neat messages", mainmenu_msgs);
  mainmenu.addItem("Multi-function buttons", mainmenu_buttons);
  mainmenu.addItem("3-button text entry", mainmenu_entry);
  mainmenu.addItem("Built-in wifi & other settings", ez.settings.menu);
  mainmenu.addItem("Updates via https", mainmenu_ota);
  mainmenu.upOnFirst("last|up");
  mainmenu.downOnLast("first|down");
  mainmenu.run();
}

void mainmenu_menus() { 
  ezMenu submenu("This is a sub menu");
  submenu.txtSmall();
  submenu.buttons("up#Back#select##down#");
  submenu.addItem("You can make small menus");
  submenu.addItem("Or big ones");
  submenu.addItem("(Like the Main menu)");
  submenu.addItem("In this menu most options");
  submenu.addItem("Do absolutely nothing");
  submenu.addItem("They are only here");
  submenu.addItem("To demonstrate that menus");
  submenu.addItem("Can run off the screen");
  submenu.addItem("And will just scroll");
  submenu.addItem("And scroll");
  submenu.addItem("And Scroll");
  submenu.addItem("And Scroll even more");   
  submenu.addItem("more | Learn more about menus", submenu_more);
  submenu.addItem("Exit | Go back to main menu");
  submenu.run();
}

enum ez_switch_t {
  SW_NORTH,
  SW_EAST,
  SW_SOUTH,
  SW_WEST
};

enum tmEntered_t {
  TM_DAY    = 0,
  TM_MONTH  = 1,
  TM_YEAR   = 2,
  TM_HOUR   = 3,
  TM_MINUTE = 4
};
  
time_t alarmTime = 0;
uint16_t updatedData = 0;
uint8_t days[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
ez_switch_t rotarySwitch = SW_NORTH;
uint8_t timeOutVar = 0;
String sRotary;
static bool toggleSwitch = false;
bool extraControl = false;

String strRotary(void){
  String sRotary;
  switch(rotarySwitch){
    case SW_NORTH:
    default:
      sRotary = "North";
      break;
    case SW_EAST:
      sRotary = "East";
      break;
    case SW_SOUTH:
      sRotary = "South";
      break;
    case SW_WEST:
      sRotary = "West";
      break;
  }
  return sRotary;
}


bool mngAlarmTime(ezMenu* callingMenu) {
  tmElements_t tmOnScreen;
  uint16_t highlight_color = TFT_RED;
  String disp_val;
  uint8_t elementSet = TM_DAY;

  //production time as start point
  tmOnScreen.Hour   = 12;
  tmOnScreen.Minute = 0;
  tmOnScreen.Day    = 1;
  tmOnScreen.Month  = 3;
  tmOnScreen.Year   = 21;

  elementSet  = TM_DAY;
  disp_val = "Enter alarm time in | DD.MM.YY HH:MM format";
  while (true) {     
    ez.msgBox("TIME", disp_val, " - # -- # select | > # OK # + # ++ # # # Back ", false, FONT_ADDR FreeMono12pt7b);
    ez.canvas.font(FONT_ADDR FreeMonoBold18pt7b);
    M5.lcd.fillRect (0, ez.canvas.bottom() - 40, TFT_W, 40, ez.theme->background); 
    ez.canvas.pos(13, ez.canvas.bottom() - 40);
    ez.canvas.color(elementSet == TM_DAY    ? highlight_color : ez.theme->msg_color);            
    ez.canvas.print(zeropad((uint32_t)tmOnScreen.Day, 2));
    ez.canvas.print(".");
    ez.canvas.color(elementSet == TM_MONTH  ? highlight_color : ez.theme->msg_color);            
    ez.canvas.print(zeropad((uint32_t)tmOnScreen.Month, 2));
    ez.canvas.color(ez.theme->msg_color);        
    ez.canvas.print(".");      
    ez.canvas.color(elementSet == TM_YEAR   ? highlight_color : ez.theme->msg_color);
    ez.canvas.print(zeropad((uint32_t)tmOnScreen.Year, 2));
    ez.canvas.color(ez.theme->msg_color);        
    ez.canvas.print(" ");
    ez.canvas.color(elementSet == TM_HOUR   ? highlight_color : ez.theme->msg_color);  
    ez.canvas.print(zeropad((uint32_t)tmOnScreen.Hour, 2));
    ez.canvas.color(ez.theme->msg_color);        
    ez.canvas.print(":");
    ez.canvas.color(elementSet == TM_MINUTE ? highlight_color : ez.theme->msg_color);          
    ez.canvas.print(zeropad((uint32_t)tmOnScreen.Minute, 2));
    String b = ez.buttons.wait();
    if (b == "select") elementSet == 4 ? elementSet-=4 : elementSet++ ; 
    switch (elementSet) {
      case TM_DAY:
        if (tmOnScreen.Day > days[tmOnScreen.Month-1]) tmOnScreen.Day = days[tmOnScreen.Month-1];
        if (b == "-" && tmOnScreen.Day >  1) tmOnScreen.Day--;
        if (b == "+" && tmOnScreen.Day < days[tmOnScreen.Month-1]) tmOnScreen.Day++;
        if (b == "--") {
          if (tmOnScreen.Day < 10) {
            tmOnScreen.Day = 0;
          } else {
            tmOnScreen.Day -= 10;
          }
        }
        if (b == "++") {
          if (tmOnScreen.Day > 20) {
            tmOnScreen.Day = days[tmOnScreen.Month-1];
          } else {
            tmOnScreen.Day += 10;
          }
        }
        break;
      case TM_MONTH: 
        if (((b == "-") || (b == "--")) && (tmOnScreen.Month >  1)) tmOnScreen.Month--;
        if (((b == "+") || (b == "++")) && (tmOnScreen.Month < 12)) tmOnScreen.Month++;
        if (tmOnScreen.Day > days[tmOnScreen.Month-1]) tmOnScreen.Day = days[tmOnScreen.Month-1];
        break;
      case TM_YEAR: 
        if (((b == "-") || (b == "--")) && (tmOnScreen.Year > 20)) tmOnScreen.Year--;
        if (((b == "+") || (b == "++")) && (tmOnScreen.Year < 35)) tmOnScreen.Year++;
        break; 
      case TM_HOUR: 
        if ((b == "-") && (tmOnScreen.Hour >  0)) tmOnScreen.Hour--;
        if ((b == "+") && (tmOnScreen.Hour < 23)) tmOnScreen.Hour++;
        if (b == "--") {
          if (tmOnScreen.Hour < 3) {
            tmOnScreen.Hour = 0;
          } else {
            tmOnScreen.Hour -= 3;
          }
        }
        if (b == "++") {
          if (tmOnScreen.Hour > 21) {
            tmOnScreen.Hour = 0;
          } else {
            tmOnScreen.Hour += 3;
          }
        }
        break;
      case TM_MINUTE: 
        if (b == "-") {
          tmOnScreen.Minute--;
          if(tmOnScreen.Minute == 255)tmOnScreen.Minute = 59;
        }
        if (b == "+") {
          tmOnScreen.Minute++;
          if(tmOnScreen.Minute == 60)tmOnScreen.Minute = 0;
        }
        if (b == "--") {
          if (tmOnScreen.Minute < 10) {
            tmOnScreen.Minute = 0;
          } else {
            tmOnScreen.Minute -= 10;
          }
        }
        if (b == "++") {
          if (tmOnScreen.Minute > 50) {
            tmOnScreen.Minute = 59;
          } else {
            tmOnScreen.Minute += 10;
          }
        }
        break;          
    }
    if (b == "OK") {
      tmOnScreen.Year += 30;
      tmOnScreen.Second = 00;
      time_t timeEntered = makeTime(tmOnScreen);
      
      if (timeStatus() == timeNotSet) {
        // timeNotSet - We are enering system time
        // store entered time somewhere
        //setRtcEpoch(timeEntered);
        UTC.setTime(timeEntered);
        ez.header.draw("clock");
        callingMenu->setCaption("settime", "System time set\t" + UTC.dateTime(timeEntered, "d.m.y H:i"));
      } else {
        // timeSet || timeNeedSync
        UTC.setTime(timeEntered);
        ez.header.draw("clock");
        callingMenu->setCaption("settime", "Alarm time set\t"  + UTC.dateTime(timeEntered, "d.m.y H:i"));
        break;
      }
      break;
    } else if (b == "Back") {
      break;
    }
  }
  return true;
}

bool mngToggle(ezMenu* callingMenu) {
    toggleSwitch = !toggleSwitch;
    callingMenu->setCaption("toggle", "Toggle something\t" + String(toggleSwitch ? "On" : "Off"));
    return true;
}

bool mngRotary(ezMenu* callingMenu) {
    uint8_t switchVar = (uint8_t)rotarySwitch;
    switchVar += 1;
    switchVar %= 4;
    rotarySwitch = (ez_switch_t)switchVar;
    callingMenu->setCaption("rotary", "Rotary Switch\t" + strRotary());
    return true;
}

bool mngTimeOut(ezMenu* callingMenu){
    timeOutVar += 15;
    timeOutVar %= 75;
    callingMenu->setCaption("timeout", "Show disabled state\t" + (String)(!timeOutVar ? "Off" : (String)(timeOutVar) + "s"));
    return true;
}

int64_t timeOnEntry, timeOnEntryOld;
uint32_t period;
uint32_t secondsCounter = 0;

uint16_t eventLoop() {
    //get some updated data 
    //for example period between loop reentries in us
    timeOnEntry = esp_timer_get_time();
    period = timeOnEntry - timeOnEntryOld;
    timeOnEntryOld = timeOnEntry;
    secondsCounter++;
    //show updated data in an item of the active menue 
    ezMenu* curMenu = M5ez::getCurrentMenu();
    if (curMenu->getTitle() == "Control") {
        curMenu->setCaption("update", "Loop period\t" + String(period) + "us");
        curMenu->setCaption("counter", "Seconds from start\t" + String(secondsCounter));
    }
    ez.yield();
    return 1000; //1s
}

bool loopStarted = false;

bool mngEventLoop(ezMenu* callingMenu) {
    callingMenu->addItem("counter | Seconds from start\t" + String(secondsCounter));
    if (!loopStarted) {
        ez.addEvent(eventLoop);
        loopStarted = true;
    }
    return true;
}


bool mainmenu_settings(ezMenu* callingMenu) {
  ezMenu controlmenu("Control");
  controlmenu.txtSmall();
  controlmenu.buttons("up#Back#select##down#");
  controlmenu.addItem("update | Start reading data\t", NULL, mngEventLoop); //to add event eventLoop() once
  controlmenu.addItem("toggle | Toggle something\t" + String(toggleSwitch ? "On" : "Off"), NULL, mngToggle);
  controlmenu.addItem("rotary | Rotary Switch\t" + strRotary(), NULL, mngRotary);
  controlmenu.addItem("timeout | Show disabled state\t" + (String)(!timeOutVar ? "Off" : (String)(timeOutVar) + "s"), NULL, mngTimeOut);
  controlmenu.addItem("settime | Set some time\t" + (String)(!alarmTime ? "dd.mm.yy hh:mm" : UTC.dateTime(alarmTime, "d.m.y H:i")), NULL, mngAlarmTime);
  controlmenu.run();
  //on Back/Exit/Done
  ez.removeEvent(eventLoop);    //if event is not needed any more
  loopStarted = false; //allows to restart eventLoop
  secondsCounter = 0;
  controlmenu.deleteItem("counter");
  return true;
}

void submenu_more() {
  ez.header.show("A simple menu in code...");
  ez.canvas.lmargin(10);
  ez.canvas.println("");
  ez.canvas.println("ezMenu menu(\"Main menu\");");
  ez.canvas.println("menu.addItem(\"Option 1\");");
  ez.canvas.println("menu.addItem(\"Option 2\");");
  ez.canvas.println("menu.addItem(\"Option 3\");");
  ez.canvas.println("while ( menu.run() ) {");
  ez.canvas.println("  if (menu.pick == 1) {");
  ez.canvas.println("    ez.msgBox (\"One!\");");
  ez.canvas.println("  }");
  ez.canvas.println("}");
  ez.buttons.wait("OK");
  
  ezMenu fontmenu("Menus can change looks");
  fontmenu.txtFont(&Satisfy_24);
  fontmenu.addItem("Menus can use");
  fontmenu.addItem("Various Fonts");
  fontmenu.runOnce();
  
  ezMenu delmenu("Menus are dynamic");
  delmenu.txtSmall();
  delmenu.addItem("You can delete items");
  delmenu.addItem("While the menu runs");
  delmenu.addItem("Delete me!");
  delmenu.addItem("Delete me!");
  delmenu.addItem("Delete me!");
  delmenu.addItem("Exit | Go back" );
  while (delmenu.runOnce()) {
    if (delmenu.pickName() == "Delete me!") {
      delmenu.deleteItem(delmenu.pick());
    }
  }
}

void mainmenu_image() {
  ezMenu images;
  images.imgBackground(TFT_BLACK);
  images.imgFromTop(40);
  images.imgCaptionColor(TFT_WHITE);
  images.addItem(sysinfo_jpg, "System Information", sysInfo);
  images.addItem(wifi_jpg, "WiFi Settings", ez.wifi.menu);
  images.addItem(about_jpg, "About M5ez", aboutM5ez);
  images.addItem(sleep_jpg, "Power Off", powerOff);
  images.addItem(return_jpg, "Back");
  images.run();
}
    
void mainmenu_msgs() {
  String cr = (String)char(13);
  ez.msgBox("You can show messages", "ez.msgBox shows text");
  ez.msgBox("Looking the way you want", "In any font !", "OK", true, FONT_ADDR FreeSerifBold24pt7b, TFT_RED);
  ez.msgBox("More ez.msgBox", "Even multi-line messages where everything lines up and is kept in the middle of the screen");
  ez.msgBox("Questions, questions...", "But can it also show any buttons you want?", "No # # Yes"); 
  ez.textBox("And there's ez.textBox", "To present or compose longer word-wrapped texts, you can use the ez.textBox function." + cr + cr + "M5ez (pronounced \"M5 easy\") is a complete interface builder library for the M5Stack ESP32 system. It allows even novice programmers to create good looking interfaces. It comes with menus as text or as images, message boxes, very flexible button setup (including different length presses and multi-button functions), 3-button text input (you have to see it to believe it) and built-in Wifi support. Now you can concentrate on what your program does, and let M5ez worry about everything else.", true);
}
    
void mainmenu_buttons() {
  ez.header.show("Simple buttons...");
  ez.canvas.font(FONT_ADDR FreeSans12pt7b);
  ez.canvas.lmargin(20);
  ez.canvas.println("");      
  ez.canvas.println("You can have three buttons");
  ez.canvas.println("with defined funtions.");
  ez.buttons.show("One # Two # Done");
  printButton();
  ez.canvas.clear();
  ez.header.show("More functions...");
  ez.canvas.println("");      
  ez.canvas.println("But why stop there?");
  ez.canvas.println("If you press a little longer");
  ez.canvas.println("You access the functions");
  ez.canvas.println("printed in cyan.");
  ez.buttons.show("One # Two # Three # Four # Done #");
  printButton();
  ez.canvas.clear();
  ez.header.show("Two keys ...");
  ez.canvas.y(ez.canvas.top() + 10);
  ez.canvas.println("It gets even better...");
  ez.canvas.println("The purple bar shows the");
  ez.canvas.println("functions for key combis.");
  ez.canvas.println("See if you can work it out...");
  ez.buttons.show("One # Two # Three # Four # Five # Six # Seven # Eight # Done");
  printButton();
}

void printButton(){
  while (true) {
  String btnpressed = ez.buttons.poll();
  if (btnpressed == "Done") break;
  if (btnpressed != "") {
    m5.lcd.fillRect (0, ez.canvas.bottom() - 45, TFT_W, 40, ez.theme->background); 
    ez.canvas.pos(20, ez.canvas.bottom() - 45);
    ez.canvas.color(TFT_RED);
    ez.canvas.font(FONT_ADDR FreeSansBold18pt7b);
    ez.canvas.print(btnpressed);
    ez.canvas.font(FONT_ADDR FreeSans12pt7b);
    ez.canvas.color(TFT_BLACK);
  }
  }
}

void mainmenu_entry() {
  if (ez.msgBox("We're gonna enter text ... !", "Have you learned to use the buttons? Go there first if you haven't been there. Or hit 'Go' to see if you can enter your name.", "Back # # Go") == "Go") {
    String your_name = ez.textInput();
    ez.msgBox("Pfew...", "Hi " + your_name + "! | | Now that was a pain! But it is good enough for entering, say, a WPA key, or don't you think?");
    ez.msgBox("Don't worry", "(You do get better with practice...)");
  }
}

void mainmenu_ota() {
  if (ez.msgBox("Get OTA_https demo", "This will replace the demo with a program that can then load the demo program again.", "Cancel#OK#") == "OK") {
    ezProgressBar progress_bar("OTA update in progress", "Downloading ...", "Abort");
    #include "raw_githubusercontent_com.h" // the root certificate is now in const char * root_cert
    if (ez.wifi.update("https://raw.githubusercontent.com/M5ez/M5ez/master/compiled_binaries/OTA_https.bin", root_cert, &progress_bar)) {
      ez.msgBox("Over The Air updater", "OTA download successful. Reboot to new firmware", "Reboot");
      ESP.restart();
    } else {
      ez.msgBox("OTA error", ez.wifi.updateError(), "OK");
    }
  }
}

void powerOff() { m5.powerOFF(); }

void aboutM5ez() {
  ez.msgBox("About M5ez", "M5ez was written by | Rop Gonggrijp | | https://github.com/M5ez/M5ez");
}
