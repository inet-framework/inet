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
    file: TransportPacket.cc
    Purpose: Base class for UDPPacket and TCPPacket Implementation
    author: Jochen Reber
*/


#include <omnetpp.h>
#include <string.h>

#include "TransportPacket.h"

// constructors
TransportPacket::TransportPacket(): cPacket()
{
	source_port_number = -1;
	destination_port_number = -1;
	msg_kind = 0;

}

TransportPacket::TransportPacket( const TransportPacket& p)
{
	setName( p.name() );
	operator=(p);
}

// no idea if this function works as expected !
TransportPacket::TransportPacket(const cMessage &msg)
{
	cMessage::operator=(msg);

	setKind(MK_PACKET);
	source_port_number = -1;
	destination_port_number = -1;
	msg_kind = msg.kind();
}

// assignment operator
TransportPacket& TransportPacket::operator=(const TransportPacket& p)
{
	cPacket::operator=(p);

	source_port_number = p.source_port_number;
	destination_port_number = p.destination_port_number;

	msg_kind = p.msg_kind;

	return *this;
}
    
// information functions
void TransportPacket::info(char *buf)
{
	cPacket::info( buf );
	sprintf( buf+strlen(buf), " Source port: %i Destination port: %i",
			source_port_number, destination_port_number);
	sprintf( buf+strlen(buf), " msg Kind: %i", msg_kind);
}

void TransportPacket::writeContents(ostream& os)
{
	os << "TransportPacket: "
		<< "\nSource port: " << source_port_number
		<< "\nDestination port: " << destination_port_number
		<< "\nMessage kind: " << msg_kind
		<< "\n";
}

