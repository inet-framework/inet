// -*- C++ -*-
// $Header$
//
// Copyright (C) 2001 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// Description:
//      Implementation of an Address Port


#include "in_port.h"

Register_Class(IN_Port);

void IN_Port::_init()
{
  _port = PORT_UNDEF;
}

IN_Port::IN_Port(const IN_Port& port) : cObject()
{
  setName(port.name());
  operator=(port);
}

IN_Port::IN_Port() : cObject()
{
  _init();
}

IN_Port::IN_Port(const char* name) : cObject(name)
{
  _init();
}

IN_Port::IN_Port(unsigned short port)
{
  _init();
  _port = port;
}

IN_Port::IN_Port(int port)
{
  _init();
 if (port > IN_Port::PORT_MAX)
    opp_error("Portnumber %d is above port limit of %d", port, IN_Port::PORT_MAX);
  else
    _port = (unsigned short) port;
}

IN_Port::~IN_Port()
{
}

void IN_Port::info(char* buf)
{
  cObject::info(buf);
  sprintf(buf + strlen(buf), " _port = %d ", (int) _port);
}

void IN_Port::writeContents(ostream& os)
{
  os << "   _port = " << _port << '\n';
}

const IN_Port& IN_Port::operator=(const IN_Port& port)
{
  if (this != & port)
    {
      cObject::operator=(port);
      _port = port.port();
    }

  return *this;
}

const IN_Port& IN_Port::operator=(const unsigned short port)
{
  _port = port;
  return *this;
}
