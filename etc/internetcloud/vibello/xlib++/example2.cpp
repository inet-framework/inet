//
// example2.cpp
//

#include <X11/Xlib.h>
#include <unistd.h>

// our stuff
#include "xlib++/display.hpp"
using namespace xlib;


main()
{
  // Open a display.
  display d("");

  if ( d )
    {
      // Create the window
      Window w = XCreateWindow((Display*)d,
			       DefaultRootWindow((Display*)d),
			       0, 0, 200, 100, 0, CopyFromParent,
			       CopyFromParent, CopyFromParent, 0, 0);

      // Show the window
      XMapWindow(d, w);
      XFlush(d);

      // Sleep long enough to see the window.
      sleep(10);
    }
  return 0;
}
