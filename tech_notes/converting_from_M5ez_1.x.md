## Converting from M5ez 1.x 

This section deals with conversion of code that was written for the 1.x versions of M5ez. Since it's now 2020 and the last 1.x was released in September of 2018, this section has been moved from the user manual to a tech note. This document is of no use if you've never used M5ez 1.x.

### What's new in M5ez 2.x ?

> *Kept here just to show what wasn't there in 1.x*

* Many user-selectable settings, including backlight, clock/timezone, FACES keyboard, etc.
* Theme values can be changed at runtime, multiple thmes can be loaded; a theme chooser then shows up in the settings
* Header can have user-defined widgets (internally used for clock and wifi signal display)
* The canvas now inherits from the Print class, so everything print and println could do, you can now also do on the canvas.
* The canvas scrolls (if you want it to)
* you can create your own events that get ran in the M5ez button wait loops
* Many bugs got fixed
* Code is a lot cleaner now, easier to add things without the sometimes ad-hoc and organically grown constructs of M5ez 1.x

However, all of this comes with some changes to the names of common functions in M5ez. Generally things are more consistent now, but that does mean they are different: your M5ez 1.x code will not run without modifications. I am not planning an M5ez 3.0 at this time, so expect no further modifications to be needed to existing code for a while.

### Converting code from M5ez 1.x to 2.x

To convert code form 1.x to 2.x, first remove the `m5.begin()` at the start of your code, and replace it by `ez.begin()`. Then it is recommended you replace the following functions:

| M5ez 1.x |    | M5ez 2.x |
|:---------|:--:|:---------|
| `ez.clearScreen()` | **&mdash;>** | `ez.screen.clear()`
| `ez.background()` | **&mdash;>** | `ez.screen.background()`
| `ez.drawHeader(` | **&mdash;>** | `ez.header.show(`
| `ez.removeHeader()` | **&mdash;>** | `ez.header.hide()`
| `ez.clearCanvas()` | **&mdash;>** | `ez.canvas.clear()`
| `ez.canvasTop()` | **&mdash;>** | `ez.canvas.top()`
| `ez.canvasBottom()` | **&mdash;>** | `ez.canvas.bottom()`
| `ez.canvasLeft()` | **&mdash;>** | `ez.canvas.left()`
| `ez.canvasRight()` | **&mdash;>** | `ez.canvas.right()`
| `ez.canvasWidth()` | **&mdash;>** | `ez.canvas.width()`
| `ez.canvasHeight()` | **&mdash;>** | `ez.canvas.height()`
| `ez.print(` | **&mdash;>** | `ez.canvas.print(`
| `ez.println(` | **&mdash;>** | `ez.canvas.println(`
| `ez.drawButtons(` | **&mdash;>** | `ez.buttons.show(`
| `ez.getButtons(` | **&mdash;>** | `ez.buttons.poll(`
| `ez.waitForButtons(` | **&mdash;>** | `ez.buttons.wait(`

Then, check all your print functions: they no longer support the setting of font, location or color in the statement itself: these need to be done with `ez.canvas.font`, `ez.canvas.color` etc. (Print is much nicer now because the canvas object inherited from the Print class, and because the canvas can scroll now.)

Then simply fix any remaining problems by first fixing any compiler complaints with this document in hand, and then walk through all of your program's functionality to see if everything works as expected.