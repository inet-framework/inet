//
// example1.cpp
//

#include <X11/Xlib.h>
#include <unistd.h>


main()
{
  // Open a display.
  Display *d = XOpenDisplay(0);


  if ( d )
    {
      // Create the window
      Window w = XCreateWindow(d, DefaultRootWindow(d), 0, 0, 200,
			       100, 0, CopyFromParent, CopyFromParent,
			       CopyFromParent, 0, 0);

      // Show the window
      XMapWindow(d, w);
      XFlush(d);

      // Sleep long enough to see the window.
      sleep(10);
    }
  return 0;
}
