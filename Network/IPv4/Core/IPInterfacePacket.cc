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
    file: IPInterfacePacket.cc
    Purpose: Message type to pass from Transport Layer
        to the IP layer; adds IP control information to
        transport packet
    Usage:
    Any transport protocol packet passed to the IP layer
    should be encapsulated into an IPInterfacePacket with
    the constructor IPInterfacePacket (cMessage *transpPacket) or
    IPInterfacePacket(cMessage *transpPacket, const char *destAddr);
    the decapsulate()-function decapsulates the transport
    protocol packet again after reception.

    comment length is set to 1!!!
    author: Jochen Reber
*/


#include <omnetpp.h>
#include <string.h>
#include <stdio.h>

#include "IPInterfacePacket.h"

// constructors
IPInterfacePacket::IPInterfacePacket(): cPacket()
{
	initValues();
}

// copy constructor
IPInterfacePacket::IPInterfacePacket(const IPInterfacePacket& ip) 
		: cPacket()
{
	setName ( ip.name() );
	operator=( ip );
}

// private function
void IPInterfacePacket::initValues()
{	
	setKind(-1);
	setLength(1);
	setPriority(0);
	setProtocol(IP_PROT_NONE);
	setSrcAddr("");
	setDestAddr("");
	setCodepoint(0);
	setTimeToLive(0);
	setDontFragment(false);
}

// assignment operator
IPInterfacePacket& IPInterfacePacket::operator=
		(const IPInterfacePacket& ip)
{
	cPacket::operator=(ip);
	
	initValues();
    strcpy (_dest, ip._dest);
    strcpy (_src, ip._src);
    _protocol = ip._protocol;
    _codepoint = ip._codepoint;
    _ttl = ip._ttl;
    _dontFragment = ip._dontFragment;

	return *this;
}

// encapsulation
void IPInterfacePacket::encapsulate(cPacket *p)
{
	cPacket::encapsulate(p);
}

// decapsulation: convert to cPacket *
cPacket *IPInterfacePacket::decapsulate()
{
	return (cPacket *)(cPacket::decapsulate());
}

// output functions
void IPInterfacePacket::info( char *buf )
{
	cPacket::info( buf );
	sprintf( buf+strlen(buf), " Prot: %i Src: %s Dest: %s",
			_protocol, _src, _dest);

}

void IPInterfacePacket::writeContents(ostream& os)
{
	os << "IPinterface: "
		<< "\nSource addr: "  << _src
		<< "\nDestination addr: " << _dest
		<< "\nProtocol: " << (int)_protocol
		<< " Codepoint: " << _codepoint
		<< " TTL: " << _ttl
		<< "\n";	
}

