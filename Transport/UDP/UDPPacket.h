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
    file: UDPPacket.h
    Purpose: Message format for UDP transport packets
    author: Jochen Reber
*/


#ifndef __UDPPACKET_H
#define __UDPPACKET_H

#include <omnetpp.h>

#include "TransportPacket.h"


/*  -------------------------------------------------
        Main class: UDPPacket 
    -------------------------------------------------
	default value: hasChecksum = true
	UDP Length = 0

	UDP packet Length is given in bytes,
	where as length() in bits .
	udpLength = 8 * length
	changing one automatically updates the other.

*/

class UDPPacket: public TransportPacket
{
private:
    bool _hasChecksum;
	int _udpLength;

public:

    // constructors
    UDPPacket();
    UDPPacket(const UDPPacket &p);
    UDPPacket(const cMessage &msg);

    // assignment operator
    virtual UDPPacket& operator=(const UDPPacket& p);
    virtual cObject *dup() const { return new UDPPacket(*this); }
    
    // info functions
    virtual void info(char *buf);
    virtual void writeContents(ostream& os);

	// overload setLength()
	virtual void setLength(int bitlength);

	bool checksumValid() const { return _hasChecksum; }
	void setChecksumValidity(bool isValid) { _hasChecksum = isValid; }

	// length of Packet in bytes
	int UDPLength() const { return _udpLength; }
	void setUDPLength(int bytelength);

};

#endif

