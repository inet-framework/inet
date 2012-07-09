//
//
// Copyright 2002 Rob Tougher <robt@robtougher.com>
//
// This file is part of xlib++.
//
// xlib++ is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// xlib++ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with xlib++; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//


// event_dispatcher

#ifndef _xlib_event_dispatcher_class_
#define _xlib_event_dispatcher_class_


#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "display.hpp"
#include <vector>
#include <algorithm>
#include "window_base.hpp"
#include "character.hpp"


namespace xlib
{

  class find_window
    {
    public:

      find_window ( Window w, window_base** p ) : m_w ( w ), m_p ( p ){};
      ~find_window(){};

      void operator () ( window_base* p )
	{
	  if ( p && p->id() == (long)m_w )
	    {
	      *m_p = p;
	    }
	}

    private:
      Window m_w;
      window_base** m_p;
    };


  class remove_window
    {
    public:
      remove_window ( window_base* p ) : m_p ( p ){}
      ~remove_window (){}

      bool operator () ( window_base* p )
	{
	  return ( p == m_p );
	}
    private:
      window_base* m_p;

    };


  class event_dispatcher
    {
    public:
      event_dispatcher ( display& d  ) : m_run ( true ), m_display ( d ){}
      ~event_dispatcher(){}


      void register_window ( window_base* p )
	{
	  if ( ! p ) return;

	  if ( std::find ( m_windows.begin(), m_windows.end(), p ) == 
	       m_windows.end() )
	    {
	      m_windows.push_back ( p );
	    }
	}

      void unregister_window ( window_base* p )
	{
	  if ( ! p ) return;

	  std::vector<window_base*>::iterator it = 
	    std::remove_if ( m_windows.begin(),
			     m_windows.end(),
			     remove_window ( p ) );

	  m_windows.erase ( it, m_windows.end() );

	}


      void run()
	{
	  m_run = true;

	  XEvent report;

	  while ( m_run )
	    {
	      XNextEvent ( m_display,
			   &report );
	      handle_event ( report );
	    }
	}


      void stop()
	{
	  m_run = false;
	}


      bool handle_event ( XEvent& report )
	{

	  window_base* p = 0;

	  std::for_each ( m_windows.begin(),
			  m_windows.end(),
			  find_window ( report.xany.window, &p ) );


	  if ( p )
	    {
	      switch ( report.type )
		{
		case Expose:
		  {
		    p->on_expose();
		    break;
		  }
		case ButtonPress:
		  {
		    if ( report.xbutton.button & Button2 )
		      {
			p->on_right_button_down ( report.xbutton.x,
						  report.xbutton.y );
		      }
		    else if ( report.xbutton.button & Button1 )
		      {
			p->on_left_button_down ( report.xbutton.x,
						 report.xbutton.y );
		      }
		    break;
		  }
		case ButtonRelease:
		  {
		    if ( report.xbutton.button & Button2 )
		      {
			p->on_right_button_up ( report.xbutton.x,
						report.xbutton.y );
		      }
		    else if ( report.xbutton.button & Button1 )
		      {
			p->on_left_button_up ( report.xbutton.x,
					       report.xbutton.y );
		      }
		    break;
		  }
		case EnterNotify:
		  {
		    p->on_mouse_enter ( report.xcrossing.x,
					report.xcrossing.y );
		    break;
		  }
		case LeaveNotify:
		  {
		    p->on_mouse_exit ( report.xcrossing.x,
				       report.xcrossing.y );
		    break;
		  }
		case MotionNotify:
		  {
		    p->on_mouse_move ( report.xmotion.x,
				       report.xmotion.y );
		    break;
		  }
		case FocusIn:
		  {
		    p->on_got_focus();
		    break;
		  }
		case FocusOut:
		  {
		    p->on_lost_focus();
		    break;
		  }
		case KeyPress:
		case KeyRelease:
		  {
		    char buf[11];
		    KeySym keysym;
		    XComposeStatus status;
		    int count = XLookupString ( &report.xkey,
						buf,
						10,
						&keysym,
						&status );

		    buf[count] = 0;

		    if ( report.type == KeyPress )
		      {
			p->on_key_press ( character(keysym,
						    buf,
						    report.xkey.state) );
		      }
		    else
		      {
			p->on_key_release ( character(keysym,
						      buf,
						      report.xkey.state) );
		      }

		    break;
		  }
		  /*
		    case DestroyNotify:
		    {
		    p->on_destroy();
		    break;
		    }
		  */

		case MapNotify:
		  {
		    p->on_show();
		    break;
		  }
		case UnmapNotify:
		  {
		    p->on_hide();
		    break;
		  }
		case ClientMessage:
		  {
		    Atom atom = XInternAtom ( m_display,
					      "WM_DELETE_WINDOW",
					      false );

		    if ( atom == report.xclient.data.l[0] )
		      {
			p->destroy();
		      }

		    break;
		  }
		}

	      return true;
	    }
	  else
	    {
	      return false;
	    }

	}


      display& get_display() { return m_display; }

    private:

      std::vector<window_base*> m_windows;
      display& m_display;
      bool m_run;

    };


};

#endif
