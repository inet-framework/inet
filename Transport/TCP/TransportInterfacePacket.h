// -*- C++ -*-
// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

/*
  file: TransportInterfacePacket.h
  Purpose: Base class for UDPPacket and TCPPacket
  author: Jochen Reber
*/


#ifndef __TRANSPORTINTERFACEPACKET_H
#define __TRANSPORTINTERFACEPACKET_H

#include <omnetpp.h>
#include "in_addr.h"

/*  -------------------------------------------------
    Main class: TransportInterfacePacket
    -------------------------------------------------
    
    msg_kind stores the kind() argument of the
    original cMessage.
    kind() itself always returns always MK_PACKET
    setKind() should not be used, use setMsgKind() instead

*/

class TransportInterfacePacket: public cPacket
{
private:
  int source_port_number;
  int destination_port_number;
  IN_Addr _srcAddr;       // currently used only by UDP
  IN_Addr _destAddr;      // currently used only by UDP
  
  int msg_kind;

public:

  // constructors
  TransportInterfacePacket();
  explicit TransportInterfacePacket(char* name);
  TransportInterfacePacket(const TransportInterfacePacket &p);
  TransportInterfacePacket(const cMessage &msg);

  // assignment operator
  virtual TransportInterfacePacket& operator=(const TransportInterfacePacket& p);
  virtual cObject *dup() const { return new TransportInterfacePacket(*this); }
    
  // info functions
  virtual void info(char *buf);
  virtual void writeContents(ostream& os);


  int sourcePort() { return source_port_number; }
  void setSourcePort(int p) { source_port_number = p; }
    
  int destinationPort() { return destination_port_number; }
  void setDestinationPort(int p) { destination_port_number = p; }

  IN_Addr sourceAddress() { return _srcAddr; }
  void setSourceAddress(const IN_Addr& srcAddr) { _srcAddr = srcAddr; }

  IN_Addr destinationAddress() { return _destAddr; }
  void setDestinationAddress(const IN_Addr& destAddr) { _destAddr = destAddr; }
    
  int msgKind() { return msg_kind; }
  void setMsgKind(int k) { msg_kind = k; }
};

#endif


