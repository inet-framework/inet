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


// shape classes

#ifndef _xlib_shape_classes_
#define _xlib_shape_classes_


namespace xlib
{

  class point
    {
    public:
      point ( int x, int y )
	: m_x ( x ),
	m_y ( y )
	{
	}
      ~point() {}

      int x() { return m_x; }
      int y() { return m_y; }

    private:
      int m_x, m_y;
    };


  class line
    {
    public:

      line ( point point1, point point2 )
	: m_point1 ( point1 ),
	m_point2 ( point2 )
	{
	}
      ~line(){}

      point point1() { return m_point1; }
      point point2() { return m_point2; }

    private:
      point m_point1, m_point2;
    };


  class rectangle
    {
    public:
      rectangle ( point origin, int width, int height )
	: m_origin ( origin ),
	m_width ( width ),
	m_height ( height )
	{
	}
      ~rectangle() {}

      point origin() { return m_origin; }
      int width() { return m_width; }
      int height() { return m_height; }

    private:
      int m_width, m_height;
      point m_origin;
    };


};

#endif
