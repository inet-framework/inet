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



// definition of the xlib::color class

#ifndef _xlib_color_class_
#define _xlib_color_class_

#include <X11/Xlib.h>
#include "exceptions.hpp"


namespace xlib
{

  class color
    {
    public:

      color ( display& d, short red, short green, short blue )
	: m_display ( d )
	{
	  m_map = DefaultColormap ( d.operator Display*(), 0 );
	  set_color ( red, green, blue );
	}


      void set ( color& c )
	{
	  free_color();
	  set_color ( c.red(), c.green(), c.blue() );
	}


      ~color()
	{
	  free_color();
	}

      long pixel() { return m_color.pixel; }
      unsigned short red() { return m_color.red * 255 / 65535; }
      unsigned short green() { return m_color.green * 255 / 65535; }
      unsigned short blue() { return m_color.blue * 255 / 65535; }


    private:

      void free_color()
	{
	  unsigned long pixels[2];
	  pixels[0] = pixel();

	  XFreeColors ( m_display,
			m_map,
			pixels,
			1,
			0 );
	}

      void set_color ( short red, short green, short blue )
	{
	  m_color.red = red * 65535 / 255;
	  m_color.green = green * 65535 / 255;
	  m_color.blue = blue * 65535 / 255;
	  m_color.flags = DoRed | DoGreen | DoBlue;

	  if ( ! XAllocColor ( m_display, m_map, &m_color ) )
	    {
	      throw create_color_exception ( "Could not create the color." );
	    }
	}

      XColor m_color;
      display& m_display;
      Colormap m_map;
    };

};


#endif
