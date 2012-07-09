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


// definition of the xlib::window class

#ifndef _xlib_window_class_
#define _xlib_window_class_

#include <string>
#include "display.hpp"
#include <X11/Xlib.h>
#include <sstream>
#include "window_base.hpp"
#include "event_dispatcher.hpp"
#include "color.hpp"
#include "shapes.hpp"


namespace xlib
{
  const int event_mask = ExposureMask | 
    ButtonPressMask | 
    ButtonReleaseMask |
    EnterWindowMask | 
    LeaveWindowMask |
    PointerMotionMask |
    FocusChangeMask |
    KeyPressMask |
    KeyReleaseMask |
    SubstructureNotifyMask |
    StructureNotifyMask |
    SubstructureRedirectMask;


  class window : public window_base
    {
    public:

      //
      // To create a window
      // <a href=constructor></a>
      window ( event_dispatcher& e, 
	       rectangle r = rectangle(point(0,0),300,200) ) 
	: m_display ( e.get_display() ),
	// m_background ( e.get_display(), 213, 206, 189 ), // sand
	//m_background ( e.get_display(), 197, 194, 197 ), // grey
	m_background ( e.get_display(), 255, 255, 255 ), // white
	m_event_dispatcher ( e ),
	m_border ( e.get_display(), 255, 255, 255 ),
	m_is_child ( false ),
	m_rect ( r ),
	m_parent ( 0 )
	{
	  m_window = 0;
	  m_atom[0] = 0;
	  show();
	}


      //
      // For existing windows
      //
      window ( event_dispatcher& e, 
	       int id ) 
	: m_display ( e.get_display() ),
	// m_background ( e.get_display(), 213, 206, 189 ), // sand
	//m_background ( e.get_display(), 197, 194, 197 ), // grey
	m_background ( e.get_display(), 255, 255, 255 ), // white
	m_event_dispatcher ( e ),
	m_border ( e.get_display(), 255, 255, 255 ),
	m_is_child ( false ),
	m_rect ( point(0,0), 0, 0 ),
	m_parent ( 0 ),
	m_window ( id )
	{
	  m_atom[0] = 0;
	}


      //
      // For a child window. 'w' is its parent.
      //
      window ( window& w  ) 
	: m_display ( w.get_event_dispatcher().get_display() ),
	m_background ( m_display, 213, 206, 189 ),
	m_event_dispatcher ( w.get_event_dispatcher() ),
	m_border ( m_display, 255, 255, 255 ),
	m_is_child ( true ),
	m_rect ( w.get_rect() ),
	m_parent ( w.id() )
	{
	  m_window = 0;
	  m_atom[0] = 0;
	  show();
	}



      virtual ~window()
	{
	  destroy();
	}

      //
      // From window_base:
      //
      virtual void show()
	{
	  create();

	  XSelectInput ( m_display,
			 m_window,
			 event_mask );

	  XMapWindow ( m_display, m_window );
	  XFlush ( m_display );
	}


      virtual void on_show(){}

      virtual void hide()
	{
	  XUnmapWindow ( m_display, m_window );
	  XFlush ( m_display );
	}


      virtual void on_hide()
	{}


      virtual void create()
	{
	  if ( ! m_window )
	    {

	      m_window = XCreateSimpleWindow ( m_display,
					       RootWindow((void*)m_display,0),
					       m_rect.origin().x(),
					       m_rect.origin().y(),
					       m_rect.width(),
					       m_rect.height(),
					       0,
					       WhitePixel((void*)m_display,0),
					       WhitePixel((void*)m_display,0));

	      set_background ( m_background );

	      if ( m_is_child )
		{

		  // keeps this window in front of its parent at all times
		  XSetTransientForHint ( m_display,
					 id(),
					 m_parent );

		  // make sure the app doesn't get killed when this
		  // window gets destroyed
		  m_atom[0] = XInternAtom ( m_display,
					    "WM_DELETE_WINDOW",
					    false );

		  XSetWMProtocols ( m_display,
				    m_window,
				    m_atom,
				    1 );
		}

	      if ( m_window == 0 )
		{
		  throw create_window_exception
		    ( "could not create the window" );
		}

	      on_create();

	    }

	  m_event_dispatcher.register_window ( this );

	}

      virtual void on_create(){}

      virtual void destroy()
	{
	  hide();

 	  if ( m_window )
	    {
	      XDestroyWindow ( m_display, m_window );
	      m_window = 0;
	    }

	  m_event_dispatcher.unregister_window ( this );

	  on_destroy();
	}


      virtual void set_background ( color& c )
	{
	  // hold a ref to the alloc'ed color
	  m_background.set ( c );

	  XSetWindowBackground ( m_display,
				 m_window,
				 c.pixel() );

	  refresh();

	}

      virtual void set_focus()
	{
	  XSetInputFocus ( m_display,
			   id(),
			   RevertToParent,
			   CurrentTime );
	  refresh();
	}


      virtual void refresh ()
	{
	  XClearWindow ( m_display,
			 m_window );

	  XFlush ( m_display );

	  on_expose();

	}


      virtual rectangle get_rect()
	{
	  Window root;
	  int x = 0, y = 0;
	  unsigned int width = 0, height = 0, border_width = 0, depth = 0;
	  XGetGeometry ( m_display,
			 m_window,
			 &root,
			 &x,
			 &y,
			 &width,
			 &height,
			 &border_width,
			 &depth );

	  return rectangle ( point(x,y), width, height );
	}


      virtual long id() { return m_window; }


      virtual void on_expose() {}

      virtual void on_left_button_down ( int x, int y ) {}
      virtual void on_right_button_down ( int x, int y ) {}

      virtual void on_left_button_up ( int x, int y ) {}
      virtual void on_right_button_up ( int x, int y ) {}

      virtual void on_mouse_enter ( int x, int y ) {};
      virtual void on_mouse_exit ( int x, int y ) {};
      virtual void on_mouse_move ( int x, int y ) {};

      virtual void on_got_focus() {};
      virtual void on_lost_focus() {};

      virtual void on_key_press ( character c ) {};
      virtual void on_key_release( character c ) {};


      virtual void on_destroy (){}

      display& get_display()
	{ return m_display; }
      virtual event_dispatcher& get_event_dispatcher() 
	{ return m_event_dispatcher; }


    private:

      //
      // Not copyable
      //
      window ( const window& );
      void operator = ( window& );

      display& m_display;

      Window m_window;
      Window m_parent;
      color m_background, m_border;
      event_dispatcher& m_event_dispatcher;

      Atom m_atom[1];

      bool m_is_child;
      rectangle m_rect;

    };

};

#endif
