//
// example5.cpp
//

#include "xlib++/display.hpp"
#include "xlib++/window.hpp"
#include "xlib++/graphics_context.hpp"
using namespace xlib;


class main_window : public window
{
 public:
  main_window ( event_dispatcher& e ) : window ( e ) {};
  ~main_window(){};

  void on_expose ()
  {
    graphics_context gc ( get_display(),
			  id() );

    gc.draw_line ( line ( point(0,0), point(50,50) ) );
    gc.draw_text ( point(0, 70), "I'm drawing!!" );
  }

};

main()
{
  try
    {
      // Open a display.
      display d("");

      event_dispatcher events ( d );
      main_window w ( events ); // top-level
      events.run();
    }
  catch ( exception_with_text& e )
    {
      std::cout << "Exception: " << e.what() << "\n";
    }
  return 0;
}
