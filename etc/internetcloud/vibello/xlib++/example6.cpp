//
// example5.cpp
//

#include "xlib++/display.hpp"
#include "xlib++/window.hpp"
#include "xlib++/graphics_context.hpp"
#include "xlib++/command_button.hpp"
using namespace xlib;
class main_window;


class hello_button : public command_button
{
public:
  hello_button ( main_window& w );
  ~hello_button(){}

  void on_click();

private:
  main_window& m_parent;
};


class main_window : public window
{
 public:
  main_window ( event_dispatcher& e )
    : window ( e )
  { m_hello = new hello_button ( *this ); }
  ~main_window(){ delete m_hello; }

  void on_hello_click() { std::cout << "hello_click()\n"; }

private:

  hello_button* m_hello;
};


//
// Hello button
//
hello_button::hello_button ( main_window& w )
  : command_button ( w, rectangle(point(20,20),100,30 ), "hello" ),
    m_parent ( w )
{}
void hello_button::on_click() { m_parent.on_hello_click(); }


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
