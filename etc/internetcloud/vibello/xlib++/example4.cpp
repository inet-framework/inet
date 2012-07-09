//
// example4.cpp
//

#include "xlib++/display.hpp"
#include "xlib++/window.hpp"
using namespace xlib;


class main_window : public window
{
 public:
  main_window ( event_dispatcher& e ) : window ( e ) {};
  ~main_window(){};
  void on_left_button_down ( int x, int y )
  {
    std::cout << "on_left_button_down()\n";
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
