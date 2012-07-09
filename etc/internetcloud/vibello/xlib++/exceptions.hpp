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

// exception classes

#ifndef _xlib_exception_classes_
#define _xlib_exception_classes_

#include <exception>

namespace xlib
{

  class exception_with_text : public std::exception
    {
    public:
      exception_with_text ( std::string what ) : m_what ( what ){};
      ~exception_with_text() throw() {};

      virtual const char* what() const throw()
	{
	  return m_what.c_str();
	}

    private:
      std::string m_what;
    };


  class open_display_exception : public exception_with_text
    {
    public:
      open_display_exception ( std::string what ) : exception_with_text ( what ){};
      ~open_display_exception() throw() {};
    };

  class create_window_exception : public exception_with_text
    {
    public:
      create_window_exception ( std::string what ) : exception_with_text ( what ){};
      ~create_window_exception() throw() {};
    };

  class create_graphics_context_exception : public exception_with_text
    {
    public:
      create_graphics_context_exception ( std::string what ) : exception_with_text ( what ){};
      ~create_graphics_context_exception() throw() {};
    };


  class create_color_exception : public exception_with_text
    {
    public:
      create_color_exception ( std::string what ) : exception_with_text ( what ){};
      ~create_color_exception() throw() {};
    };

  class create_button_exception : public exception_with_text
    {
    public:
      create_button_exception ( std::string what ) : exception_with_text ( what ){};
      ~create_button_exception() throw() {};
    };

  class create_edit_exception : public exception_with_text
    {
    public:
      create_edit_exception ( std::string what ) : exception_with_text ( what ){};
      ~create_edit_exception() throw() {};
    };
};


#endif
