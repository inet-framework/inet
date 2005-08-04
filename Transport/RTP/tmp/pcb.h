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
//      Definition of a Generic Protocol Control Block
//      In fact an abstract definition of netinet/in_pcb.h
//
//      Instead of having a pointer pointing to a "per-protocol PCB",
//      inheritance should be used, e.g. for a TCP TCB.

// Information common to all to all UDP and TCP endpoints:
// (note: not necessarily all of them are implemented here)
// - foreign and local IP address
// - foreign and local port numbers
// - IP header prototype
// - IP options to use for this end point
// - pointer to routing table


#ifndef PCB_H
#define PCB_H

#include <omnetpp.h>

//#include "in_addr.h"
//#include "in_port.h"

class INET_API PCB : public cObject
{
 private:

  IN_Addr _faddr;       // foreign address
  IN_Port _fport;       // foreign port
  IN_Addr _laddr;       // local address
  IN_Port _lport;       // local port

  // private member functions
  void _init();

 public:

  // creation, duplication, destruction
  PCB(const PCB& pcb);
  PCB(IN_Addr laddr, IN_Port lport, IN_Addr faddr, IN_Port fport);
  PCB();
  explicit PCB(const char* name);
  //PCB(const char* name, cOjbect* ownerobj);
  virtual ~PCB();
  virtual cObject* dup() const {return new PCB(*this);}
  virtual void info(char* buf);
  virtual void writeContents(std::ostream& os);
  PCB& operator=(const PCB& pcb);

  // new member functions
  const IN_Addr& fAddr() const {return _faddr;}
  const IN_Addr& lAddr() const {return _laddr;}

  const IN_Port& fPort() const {return _fport;}
  const IN_Port& lPort() const {return _lport;}

  void setFAddr(const IN_Addr& addr) {_faddr = addr;}
  void setLAddr(const IN_Addr& addr) {_laddr = addr;}

  void setFPort(const IN_Port& port) {_fport = port;}
  void setLPort(const IN_Port& port) {_lport = port;}

};

#endif // PCB_H
