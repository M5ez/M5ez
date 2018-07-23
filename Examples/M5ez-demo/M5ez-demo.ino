#include <M5Stack.h>
#include <M5ez.h>

#include "images.h"

#define MAIN_DECLARED   true

void setup() {

  m5.begin();

}

void loop() {
  ezMenu main("Welcome to M5ez");
  main.addItem("Flexible text menus", main_menus);
  main.addItem("Image menus", main_image);
  main.addItem("Neat messages", main_msgs);
  main.addItem("Multi-function buttons", main_buttons);
  main.addItem("3-button text entry", main_entry);
  main.addItem("Built-in WiFi support", ezWifiMenu);
  main.upOnFirst("last|up");
  main.downOnLast("first|down");
  main.run(); 
}

void main_menus() { 
  ezMenu submenu("This is a sub menu");
  submenu.txtSmall();
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

void submenu_more() {
  ez.drawHeader("A simple menu in code...");
  ez.printLmargin(10);
  ez.println("");
  ez.println("ezMenu main(\"Main menu\");");
  ez.println("main.addItem(\"Option 1\");");
  ez.println("main.addItem(\"Option 2\");");
  ez.println("main.addItem(\"Option 3\");");
  ez.println("while ( main.run() ) {");
  ez.println("  if (main.pick == 1) {");
  ez.println("    ez.msgBox (\"One!\");");
  ez.println("  }");
  ez.println("}");
  ez.waitForButtons("OK");

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

void main_image() {
  ezMenu images;
  images.imgBackground(TFT_BLACK);
  images.imgFromTop(40);
  images.imgCaptionFont(&FreeSansBold12pt7b);
  images.imgCaptionLocation(TC_DATUM);
  images.imgCaptionColor(TFT_WHITE);
  images.imgCaptionMargins(10,10);
  images.addItem(sysinfo_jpg, "System Information", sysInfo);
  images.addItem(wifi_jpg, "WiFi Settings", ezWifiMenu);
  images.addItem(about_jpg, "About M5ez", aboutM5ez);
  images.addItem(sleep_jpg, "Power Off", powerOff);
  images.addItem(return_jpg, "Back");
  images.run();
}
    
void main_msgs() {
  ez.msgBox("You can show messages", "ez.msgBox shows text");
  ez.msgBox("Looking the way you want", "In any font !", "OK", true, &FreeSerifBold24pt7b, TFT_RED);
  ez.msgBox("More ez.msgBox", "Even multi-line messages | where everything lines up | and is kept in the middle | of the screen");
  ez.msgBox("Questions, questions...", "But can it also show | any buttons you want?", "No # # Yes"); 
}
    
void main_buttons() {
  ez.drawHeader("Simple buttons...");
  ez.printFont(&FreeSans12pt7b);
  ez.printLmargin(20);
  ez.println("");      
  ez.println("You can have three buttons");
  ez.println("with defined funtions.");
  ez.drawButtons("One # Two # Done");
  printButton();
  ez.clearScreen();
  ez.drawHeader("More functions...");
  ez.printFont(&FreeSans12pt7b);
  ez.printLmargin(20);
  ez.println("");      
  ez.println("But why stop there?");
  ez.println("If you press a little longer");
  ez.println("You access the functions");
  ez.println("printed in cyan.");
  ez.drawButtons("One # Two # Three # Four # Done #");
  printButton();
  ez.clearScreen();
  ez.drawHeader("Two keys ...");
  ez.printFont(&FreeSans12pt7b);
  ez.printLmargin(20);     
  ez.println("It gets even better...", 20, ez.canvasTop() + 10);
  ez.println("The purple bar shows the");
  ez.println("functions for key combis.");
  ez.println("See if you can work it out...");
  ez.drawButtons("One # Two # Three # Four # Five # Six # Seven # Eight # Done");
  printButton();
}

void printButton(){
  while (true) {
    String btnpressed = ez.getButtons();
    if (btnpressed == "Done") break;
    if (btnpressed != "") {
        m5.lcd.fillRect (0, ez.canvasBottom() - 45, TFT_W, 40, SCREEN_BGCOLOR); 
        ez.print(btnpressed, 20, ez.canvasBottom() - 45, &FreeSansBold18pt7b, TFT_RED);
    }
  }
}

void main_entry() {
  if (ez.msgBox("We're gonna enter text ... !", "Have you learned to use| the buttons? Go there first | if you haven't been there. | Or hit 'Go' to see if you | can enter your name.", "Back # # Go") == "Go") {
    String your_name = ez.textInput();
    ez.msgBox("Pfew...", "Hi " + your_name +" !| | Now that was a pain ! | But it is good enough for | entering, say, a WPA key, | or don't you think?");
    ez.msgBox("Don't worry", "(You do get better | with practice...)");
  }
}

void powerOff() { m5.powerOFF(); }

void aboutM5ez() {
  ez.msgBox("About M5ez", "M5ez was written by | Rop Gonggrijp | | https://github.com/ropg/M5ez");
}

