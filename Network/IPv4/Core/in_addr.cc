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
//      Implementation of an Internet Address

#include "in_addr.h"

const char* const IN_Addr::ADDR_UNDEF = "";

Register_Class(IN_Addr);

void IN_Addr::_init()
{
  _addrstr[0] = '\0';
}

void IN_Addr::_setaddr(const char* addr)
{
  sprintf(&_addrstr[0], "%s", addr);
}

// creation, copying, destruction
IN_Addr::IN_Addr(const IN_Addr& addr) : cObject()
{
  operator=(addr);
}

IN_Addr::IN_Addr() : cObject()
{
  _init();
}

IN_Addr::IN_Addr(const char* addr) : cObject(addr)
{
  setName(addr);
  _setaddr(addr);
}

IN_Addr::~IN_Addr()
{
}

void IN_Addr::info(char* buf)
{
  cObject::info(buf);
  sprintf(buf + strlen(buf), " _addrstr = %s", &_addrstr[0]);
}

void IN_Addr::writeContents(ostream& os)
{
  os << " _addrstr = " << &_addrstr[0] << '\n';
}

IN_Addr& IN_Addr::operator=(const IN_Addr& addr)
{
  if (this != &addr)
    {
      cObject::operator=(addr);
      setName(addr.name());
      _setaddr(addr);
    }
  return *this;
}

IN_Addr& IN_Addr::operator=(const char* addr)
{
  setName(addr);
  _setaddr(addr);
  return *this;
}

IN_Addr::operator const char* () const
{
  return &_addrstr[0];
}

void IN_Addr::setAddr(const char* addr)
{
  _setaddr(addr);
}

void IN_Addr::setAddr(const IN_Addr& addr)
{
  operator=(addr);
}
