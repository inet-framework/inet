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
    file: UDPPacket.cc
    Purpose: standard functions and fields for UDP message class
    author: Jochen Reber
*/


#include <omnetpp.h>
#include <string.h>

#include "UDPPacket.h"

// constructors
UDPPacket::UDPPacket(): TransportPacket()
{
	_hasChecksum = true;
	setUDPLength(0);

}

UDPPacket::UDPPacket( const UDPPacket& p): TransportPacket(p)
{
	setName( p.name() );
	operator=(p);
}

// function needs to be revised. Some error in the assignment!
// function might not be required.
UDPPacket::UDPPacket(const cMessage &msg): TransportPacket(msg)
{
	setUDPLength(msg.length());
	setChecksumValidity(true);
}

// assignment operator
UDPPacket& UDPPacket::operator=(const UDPPacket& p)
{
	TransportPacket::operator=(p);

	setUDPLength(p.UDPLength());
	setChecksumValidity(p.checksumValid());

	return *this;
	
}
    
// information functions
void UDPPacket::info(char *buf)
{
	TransportPacket::info( buf );
	sprintf( buf+strlen(buf), " UDP Packet Length: %i Checksum valid: %s",
			UDPLength(), _hasChecksum ? "true" : "false");
}

void UDPPacket::writeContents(ostream& os)
{
	os << "UDPPacket: "
		<< "\nSource port: " << sourcePort()
		<< "\nDestination port: " << destinationPort()
		<< "\nMessage kind: " << msgKind()
		<< "\nUDP Packet Length: " << UDPLength()
		<< "\nUDP checksum valid: " << (_hasChecksum ? "true" : "false")
		<< "\n";
}

void UDPPacket::setLength(int bitlength)
{
	TransportPacket::setLength(bitlength);
	_udpLength = bitlength / 8;
}

void UDPPacket::setUDPLength(int bytelength)
{
	TransportPacket::setLength(bytelength * 8);
	_udpLength = bytelength;
}

