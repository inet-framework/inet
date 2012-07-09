//
// example3.cpp
//

#include "xlib++/display.hpp"
#include "xlib++/window.hpp"
using namespace xlib;


class main_window : public window
{
 public:
  main_window ( event_dispatcher& e ) : window ( e ) {};
  ~main_window(){};
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
