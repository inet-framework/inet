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


// graphics_context class

#ifndef _xlib_graphics_context_class_
#define _xlib_graphics_context_class_

#include "display.hpp"
#include "window.hpp"
#include "exceptions.hpp"
#include "color.hpp"
#include "shapes.hpp"
#include <vector>


namespace xlib
{

  class graphics_context
    {
    public:
      graphics_context ( display& d,
			 int window_id )
	: m_display ( d ),
	m_window_id ( window_id )
	{
	  XGCValues values;
	  values.background = 1;
	  m_gc = 0;
	  m_gc = XCreateGC ( m_display,
			     m_window_id,
			     GCBackground,
			     &values );

	  if ( m_gc == 0 )
	    {
	      throw create_graphics_context_exception
		( "Could not create the graphics context." );
	    }
	};

      ~graphics_context(){};


      // drawing primitives

      void draw_line ( line l )
	{
	  XDrawLine ( m_display,
		      m_window_id,
		      m_gc,
		      l.point1().x(),
		      l.point1().y(),
		      l.point2().x(),
		      l.point2().y() );
	}


      void draw_rectangle ( rectangle rect )
	{
	  XDrawRectangle ( m_display,
			   m_window_id,
			   m_gc,
			   rect.origin().x(),
			   rect.origin().y(),
			   rect.width(),
			   rect.height() );
	}

      void draw_text ( point origin, std::string text )
	{
	  XDrawString ( m_display,
			m_window_id,
			m_gc,
			origin.x(),
			origin.y(),
			text.c_str(),
			text.size() );
	}

      void fill_rectangle ( rectangle rect )
	{
	  XFillRectangle ( m_display,
			   m_window_id,
			   m_gc,
			   rect.origin().x(),
			   rect.origin().y(),
			   rect.width(),
			   rect.height() );
	}


      void set_foreground ( color& c )
	{
	  XSetForeground ( m_display,
			   m_gc,
			   c.pixel() );
	}

      void set_background ( color& c )
	{
	  XSetBackground ( m_display,
			   m_gc,
			   c.pixel() );
	}

      rectangle get_text_rect ( std::string text )
	{
	  int direction = 0, font_ascent = 0, font_descent = 0;
	  XCharStruct char_struct;

	  XQueryTextExtents ( m_display,
			      XGContextFromGC(m_gc),
			      text.c_str(),
			      text.size(),
			      &direction,
			      &font_ascent,
			      &font_descent,
			      &char_struct );

	  rectangle rect ( point(0,0),
			   char_struct.rbearing - char_struct.lbearing,
			   char_struct.ascent - char_struct.descent );

	  return rect;

	}


      std::vector<int> get_character_widths ( std::string text )
	{

	  // GContext gc_id = XGContextFromGC ( (GC)gc.id() );

	  std::vector<int> char_widths;

	  XFontStruct * font = XQueryFont ( m_display, (GContext)id() );

	  for ( std::string::const_iterator it = text.begin();
		it != text.end();
		it++ )
	    {
	      std::string temp;
	      //temp += it;
	      temp += *it;

	      int width = XTextWidth ( font,
				       temp.c_str(),
				       1 );

	      char_widths.push_back ( width );
	    }

	  return char_widths;
	}

      int get_text_height ()
	{
	  XFontStruct * font = XQueryFont ( m_display, (GContext)id() );

	  if ( font )
	    {
	      return font->max_bounds.ascent + font->max_bounds.descent;
	    }
	  else
	    {
	      return 0;
	    }
	}

      long id() { return XGContextFromGC(m_gc); }

    private:

      display& m_display;
      int m_window_id;
      GC m_gc;
    };

};



#endif
