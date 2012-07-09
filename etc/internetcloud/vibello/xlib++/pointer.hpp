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


// pointer class

#ifndef _xlib_pointer_class_
#define _xlib_pointer_class_


namespace xlib
{

  class pointer
    {
    public:
      pointer ( display& d )
	: m_display ( d )
	{
	  unsigned int mask;
	  if ( XQueryPointer ( m_display,
			       RootWindow((void*)m_display,0),
			       &m_root,
			       &m_child,
			       &m_root_x,
			       &m_root_y,
			       &m_root_x,
			       &m_root_y,
			       &mask ) )
	    {

	      m_left_button_down = mask & Button1Mask;
	      m_right_button_down = mask & Button2Mask;


	      XTranslateCoordinates ( m_display,
				      RootWindow((void*)m_display,0),
				      m_child,
				      m_root_x,
				      m_root_y,
				      &m_child_x,
				      &m_child_y,
				      &m_child );

	    }
	  else
	    {
	      m_left_button_down = false;
	      m_right_button_down = false;
	    }

	}

      ~pointer() {}

      Window get_window() { return m_child; }
      bool is_left_button_down() { return m_left_button_down; }
      bool is_right_button_down() { return m_right_button_down; }


    private:
      display& m_display;
      int m_root_x, m_root_y, m_child_x, m_child_y;
      Window m_root, m_child;
      bool m_left_button_down, m_right_button_down;
    };


};

#endif
