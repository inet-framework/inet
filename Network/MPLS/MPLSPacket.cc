/***************************************************************************
*
*	This library is free software, you can redistribute it and/or modify 
*	it under  the terms of the GNU Lesser General Public License 
*	as published by the Free Software Foundation; 
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful, 
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*	See the GNU Lesser General Public License for more details.
*
*
***************************************************************************/



/*
*	File Name MPLSPacket.cc
*	MPLS library
*	This file implements MPLSPacket class
**/

#include <omnetpp.h>
#include <string.h>

#include "MPLSPacket.h"

// constructors
MPLSPacket::MPLSPacket(): cPacket()
{
	setLength(8 * MPLS_HEADER_LENGTH);

	_protocol = MPLS_PROT_UNDEF;
}

MPLSPacket::MPLSPacket(const MPLSPacket& p)
{
	setName( p.name() );
	operator=(p);
}

// assignment operator
MPLSPacket& MPLSPacket::operator=(const MPLSPacket& p)
{
	cPacket::operator=(p);
	_protocol = p._protocol;
	return *this;
}
    
// information functions
void MPLSPacket::info(char *buf)
{
	cPacket::info( buf );
	sprintf( buf+strlen(buf), " Protocol: %x",
			_protocol);
}

void MPLSPacket::writeContents(ostream& os)
{
	os << "MPLSPacket: "
		<< "\nProtocol " << (int)_protocol
		<< "\n";
}



