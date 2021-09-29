/*
 * 
 * This is a "z-sketch". It means you can run this sketch on its own, or use it as a sub-sketch of some bigger program
 * See the M5ez user manual under z-sketches at https://github.com/M5ez/M5ez
 * 
 * This program sets the ezTime NTP interval to 20 seconds (way too often, use only for demonstration purposes) and
 * then show the debug log of ezTime so you can see it update its clock. The output will scroll when it reaches the
 * bottom of the screen.
 * 
 */


#ifndef MAIN_DECLARED

#include <M5Stack.h>
#include <M5ez.h>
#include <ezTime.h>

void setup() {
  ez.begin();
  ezTimeLog();

  while(true) {
    ez.yield();
  }
}

void loop() {

}

#endif  // #ifndef MAIN_DECLARED



void ezTimeLog() {

  ez.header.show("ezTime log");
  ez.canvas.scroll(true);
  ezt::setDebug(INFO, ez.canvas);
  ezt::setInterval(20);

  #ifdef MAIN_DECLARED
    ez.buttons.wait("Abort");
    ezt::setDebug(NONE, Serial);
    ezt::setInterval(600);
  #endif
  
}
