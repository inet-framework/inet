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

#ifndef _xlib_character_class_
#define _xlib_character_class_

#include <X11/Xlib.h>
#include <X11/keysym.h>


namespace xlib
{

  class character
    {
    public:
      character ( long keysym, std::string text, long state ) 
	: m_key ( keysym ), m_text ( text ), m_state ( state )
	{
	}

      ~character (){}

      bool is_delete_key() { return m_key == XK_Delete; }
      bool is_backspace_key() { return m_key == XK_BackSpace; }
      bool is_left_arrow_key() { return m_key == XK_Left || m_key == XK_KP_Left; }
      bool is_right_arrow_key() { return m_key == XK_Right || m_key == XK_KP_Right; }


      bool is_printable()
	{
	  return ( ( ( m_key >= XK_KP_Space ) && ( m_key <= XK_KP_9 ) )
		   || ( ( m_key >= XK_space ) && ( m_key <= XK_asciitilde ) ) );
	}

      bool is_shift_key_pressed()
	{
	  return (m_state & ShiftMask);
	}

      std::string get_text()
	{
	  if ( ! is_printable() ) return std::string ( "" );
	  return m_text;
	}


    private:

      long m_key;
      std::string m_text;
      long m_state;
    };

};


#endif
