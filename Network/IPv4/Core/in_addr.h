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
//      Definition of an Internet Address

#ifndef IN_ADDR_H
#define IN_ADDR_H

#include <string.h>
#include <omnetpp.h>
#include "basic_consts.h"

class IN_Addr : public cObject
{
 private:
  char _addrstr[NODE_NAME_SIZE];        // something like "127.0.0.1"
  
  void _init();
  void _setaddr(const char* addr);

public:
  const static char* const ADDR_UNDEF;
  IN_Addr(const IN_Addr& addr);
  IN_Addr();
  IN_Addr(const char* addr);
  //IN_Addr(const char* name, cOjbect* ownerobj);
  virtual ~IN_Addr();
  virtual cObject* dup() const {return new IN_Addr(*this);}
  virtual void info(char* buf);
  virtual void writeContents(ostream& os);
  IN_Addr& operator=(const IN_Addr& addr);

  // new member functions
  IN_Addr& operator=(const char* addr);
  bool operator==(const IN_Addr& addr) const
  {return (strcmp(&_addrstr[0], (const char*) addr) == 0);}
  bool operator!=(const IN_Addr& addr) const
  {return !operator==(addr);}
  operator const char* () const;

  void setAddr(const char* addr);
  void setAddr(const IN_Addr& addr);
  bool isValid() const {return _addrstr[0] != '\0';}
};

#endif // IN_ADDR_H
