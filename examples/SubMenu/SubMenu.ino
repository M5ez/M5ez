#include <M5ez.h>

// This example is intended to show how to nest menus within menus, even when
// special button processing is needed on menu selections.


// Standard Arduino setup function. ez.begin() takes care of startup and initialization
void setup() {
  #include <themes/default.h>                         // per https://github.com/M5ez/M5ez#themes
  #include <themes/dark.h>                            // makes both themes available
  ez.begin();                                         // initializes everything
}


// Although the Arduino loop is usually called repeatedly, using an M5ez menu with no exit results in it being called once only
//
void loop() {
  Serial.println("entering loop()");                  // Shows what function we're in on the Serial Monitor
  ezMenu topmenu("Top Menu");                         // creates the menu but does nothing else
  topmenu.txtSmall();                                 // makes menu text smaller, neater
  topmenu.buttons("up # select # down");              // 'select' sets pickName and pickCaption, all set pickButton
  topmenu.addItem("dark | Dark Theme",  dark);        // ignore in while loop, calls dark() automatically
  topmenu.addItem("light | Light Theme", light);      // ignore in while loop, calls light() automatically
  topmenu.addItem("Red");                             // does nothing; handled manually in while loop
  topmenu.addItem("Green");                           // does nothing; handled manually in while loop
  topmenu.addItem("Blue");                            // does nothing; handled manually in while loop
  topmenu.addItem("Black");                            // does nothing; handled manually in while loop
  topmenu.addItem("SubMenu");                         // does nothing; handled manually in while loop
  while(topmenu.runOnce()) {                          // runOnce displays the menu and returns when a button is pressed
    // you can watch the results in the Serial Monitor
    Serial.printf("topmenu.pick        = %d\n",   topmenu.pick());
    Serial.printf("topmenu.pickButton  = %s\n",   topmenu.pickButton().c_str());
    Serial.printf("topmenu.pickName    = %s\n",   topmenu.pickName().c_str());
    Serial.printf("topmenu.pickCaption = %s\n\n", topmenu.pickCaption().c_str());
    
    String result  = topmenu.pickName();              // find out what menu was selected when 'select' button was pressed
    if(     result == "Red")     ez.theme->menu_item_color = RED;
    else if(result == "Green")   ez.theme->menu_item_color = GREEN;
    else if(result == "Blue")    ez.theme->menu_item_color = BLUE;
    else if(result == "Black")   ez.theme->menu_item_color = BLACK;
    else if(result == "SubMenu") subMenu();           // create a sub menu, but we can return to this menu when it exits
  }
}


// Function called automatically by selecting the 'Dark Theme' menu
//
void dark() { ez.theme->select("Dark"); }             // called automatically when 'Dark Theme' menu is selected


// Function called automatically by selecting the 'Light Theme' menu
//
void light() { ez.theme->select("Default"); }         // called automatically when 'Light Theme' menu is selected


// Display the submenu. When 'back' is selected, this menu is terminated and mainmenu resumes
// the foo, bar and baz menus are just dummies; normally they would call a function of be handled in a runOnce loop
//
void subMenu() {
  Serial.println("entering subMenu()");               // Shows what function we're in on the Serial Monitor
  ezMenu submenu("Sub Menu");                         // creates the menu but does nothing else
  submenu.txtSmall();                                 // makes menu text smaller, neater
  submenu.buttons("up # back # select ## down #");    // standard buttons for a submenu; long press on button A pops up one level
  submenu.addItem("foo");                             // not handled at all, so nothing happens when this menu is selected except a redraw
  submenu.addItem("bar");                             // not handled at all, so nothing happens when this menu is selected except a redraw
  submenu.addItem("baz");                             // not handled at all, so nothing happens when this menu is selected except a redraw
  submenu.addItem("sub | subSubMenu");                // returns the name "sub" but displays the caption "subSubMenu"
  while(submenu.runOnce()) {                          // runOnce displays the menu and returns when a button is pressed
    // you can watch the results in the Serial Monitor
    Serial.printf("submenu.pick        = %d\n",   submenu.pick());
    Serial.printf("submenu.pickButton  = %s\n",   submenu.pickButton().c_str());
    Serial.printf("submenu.pickName    = %s\n",   submenu.pickName().c_str());
    Serial.printf("submenu.pickCaption = %s\n\n", submenu.pickCaption().c_str());
    
    if(submenu.pickName() == "sub") subSubMenu();     // submenu.pickName() == "sub", or submenu.pickCaption() == "subSubMenu", or submenu.pick == 4
  }
}


// Display an even more deaply nested menu. When 'back' is selected this menu is terminated and submenu resumes
// the tic, tac and toe menus are just dummies; normally they would call a function of be handled in a runOnce loop
//
void subSubMenu() {
  Serial.println("entering subSubMenu()");            // Shows what function we're in on the Serial Monitor
  ezMenu subsubmenu("Sub Sub Menu");                  // creates the menu but does nothing else
  subsubmenu.txtSmall();                              // makes menu text smaller, neater
  subsubmenu.buttons("up # back # select ## down #"); // standard buttons for a submenu; long press on button A pops up one level
  subsubmenu.addItem("tic");                          // not handled at all, so nothing happens when this menu is selected except a redraw
  subsubmenu.addItem("tac");                          // not handled at all, so nothing happens when this menu is selected except a redraw
  subsubmenu.addItem("toe");                          // not handled at all, so nothing happens when this menu is selected except a redraw
  subsubmenu.run();                                   // display menu and handle menus until 'back' is selected, then exit, returning to subMenu()
}
