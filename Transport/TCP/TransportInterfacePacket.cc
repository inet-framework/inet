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
    file: TransportInterfacePacket.cc
    Purpose: Base class for UDPPacket and TCPPacket Implementation
    author: Jochen Reber
*/


#include <omnetpp.h>
#include <string.h>

#include "TransportInterfacePacket.h"

// constructors
TransportInterfacePacket::TransportInterfacePacket(): cPacket()
{
	source_port_number = -1;
	destination_port_number = -1;
	msg_kind = 0;

}

TransportInterfacePacket::TransportInterfacePacket(char* name): cPacket(name)
{
  source_port_number = -1;
  destination_port_number = -1;
  msg_kind = 0;
}

TransportInterfacePacket::TransportInterfacePacket(const TransportInterfacePacket& p)
{
	setName( p.name() );
	operator=(p);
}

// no idea if this function works as expected !
TransportInterfacePacket::TransportInterfacePacket(const cMessage &msg)
{
	cMessage::operator=(msg);

	setKind(MK_PACKET);
	source_port_number = -1;
	destination_port_number = -1;
	msg_kind = msg.kind();
}

// assignment operator
TransportInterfacePacket& TransportInterfacePacket::operator=(const TransportInterfacePacket& p)
{
	cPacket::operator=(p);

	source_port_number = p.source_port_number;
	destination_port_number = p.destination_port_number;

	msg_kind = p.msg_kind;

	return *this;
}
    
// information functions
void TransportInterfacePacket::info(char *buf)
{
	cPacket::info( buf );
	sprintf( buf+strlen(buf), " Source %s %i Destination %s %i",
                 (const char*) _srcAddr, source_port_number,
                 (const char*) _destAddr, destination_port_number);
	sprintf( buf+strlen(buf), " msg Kind: %i", msg_kind);
}

void TransportInterfacePacket::writeContents(ostream& os)
{
	os << "TransportInterfacePacket: "
           << "\nSource Address: " << (const char*) _srcAddr
           << "\nSource port: " << source_port_number
           << "\nDestination Address: " << (const char*) _destAddr
           << "\nDestination port: " << destination_port_number
           << "\nMessage kind: " << msg_kind
           << "\n";
}

