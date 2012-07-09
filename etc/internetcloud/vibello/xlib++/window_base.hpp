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


// definition of the xlib::window_base class

#ifndef _xlib_window_base_class_
#define _xlib_window_base_class_

#include <X11/Xlib.h>
#include "color.hpp"
#include "shapes.hpp"
#include "character.hpp"

namespace xlib
{

  class event_dispatcher;
  class display;

  class window_base
    {
    public:

      virtual long id() = 0;


      virtual void show() = 0;
      virtual void hide() = 0;
      virtual void create() = 0;
      virtual void destroy() = 0;
      virtual void refresh() = 0;

      virtual void set_background ( color& c ) = 0;

      virtual void set_focus() = 0;

      virtual rectangle get_rect() = 0;
      virtual event_dispatcher& get_event_dispatcher() = 0;
      virtual display& get_display() = 0;


      // callbacks
      virtual void on_expose() = 0;

      virtual void on_show() = 0;
      virtual void on_hide() = 0;

      virtual void on_left_button_down ( int x, int y ) = 0;
      virtual void on_right_button_down ( int x, int y ) = 0;

      virtual void on_left_button_up ( int x, int y ) = 0;
      virtual void on_right_button_up ( int x, int y ) = 0;

      virtual void on_mouse_enter ( int x, int y ) = 0;
      virtual void on_mouse_exit ( int x, int y ) = 0;
      virtual void on_mouse_move ( int x, int y ) = 0;

      virtual void on_got_focus() = 0;
      virtual void on_lost_focus() = 0;

      virtual void on_key_press ( character c ) = 0;
      virtual void on_key_release ( character c ) = 0;

      virtual void on_create() = 0;
      virtual void on_destroy() = 0;

    };

};

#endif
