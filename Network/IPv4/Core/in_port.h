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
//      Definition of an Address Port

#ifndef IN_PORT_H
#define IN_PORT_H

#include <string.h>
#include <omnetpp.h>

class IN_Port : public cObject
{
 private:
  unsigned short _port;

  void _init();
 public:

  enum {PORT_UNDEF = 0 };
  enum {PORT_MAX = 0x7fff };
  
  // creation, duplication, destruction
  IN_Port(const IN_Port& port);
  IN_Port(unsigned short port);
  IN_Port(int port);
  IN_Port();
  explicit IN_Port(const char* name);
  //IN_Port(const char* name, cOjbect* ownerobj);
  virtual ~IN_Port();
  virtual void info(char* buf);
  virtual void writeContents(ostream& os);
  virtual cObject* dup() const {return new IN_Port(*this);}
  const IN_Port& operator=(const IN_Port& port);
  const IN_Port& operator=(const unsigned short port);
  
  // new member functions
  bool operator==(const IN_Port& port) const
  {return (_port == port.port());}
  bool operator!=(const IN_Port& port) const
  {return !operator==(port);}
  unsigned short port() const {return _port;}
  operator const int () const {return _port;}
  void setPort(unsigned short port) {_port = port;}
  bool isValid() const {return _port != PORT_UNDEF;}
};

#endif
