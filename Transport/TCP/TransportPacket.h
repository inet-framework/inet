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
    file: TransportPacket.h
    Purpose: Base class for UDPPacket and TCPPacket
    author: Jochen Reber
*/


#ifndef __TRANSPORTPACKET_H
#define __TRANSPORTPACKET_H

#include <iostream>
#include <omnetpp.h>

using std::ostream;

/*  -------------------------------------------------
        Main class: TransportPacket
    -------------------------------------------------

    msg_kind stores the kind() argument of the
    original cMessage.
    kind() itself always returns always MK_PACKET
    setKind() should not be used, use setMsgKind() instead

*/

class TransportPacket: public cPacket
{
private:
    int source_port_number;
    int destination_port_number;

    int msg_kind;

public:

    // constructors
    TransportPacket();
    TransportPacket(const TransportPacket &p);
    TransportPacket(const cMessage &msg);

    // assignment operator
    virtual TransportPacket& operator=(const TransportPacket& p);
    virtual cObject *dup() const { return new TransportPacket(*this); }

    // info functions
    virtual void info(char *buf);
    virtual void writeContents(ostream& os);


    int sourcePort() const { return source_port_number; }
    void setSourcePort(int p) { source_port_number = p; }

    int destinationPort() const { return destination_port_number; }
    void setDestinationPort(int p) { destination_port_number = p; }

    int msgKind() const { return msg_kind; }
    void setMsgKind(int k) { msg_kind = k; }
};

#endif


