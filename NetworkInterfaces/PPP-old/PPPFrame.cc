//
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
    file: PPPFrame .cc
    Purpose: PPP frame format definition Implementation
    author: Jochen Reber
*/


#include <omnetpp.h>
#include <string.h>

#include "PPPFrame.h"

// constructors
PPPFrame::PPPFrame(): cPacket()
{
    setLength(8 * PPP_HEADER_LENGTH);
    _protocol = PPP_PROT_UNDEF;
}

PPPFrame::PPPFrame(const PPPFrame& p)
{
    setName( p.name() );
    operator=(p);
}

// assignment operator
PPPFrame& PPPFrame::operator=(const PPPFrame& p)
{
    cPacket::operator=(p);
    _protocol = p._protocol;
    return *this;
}

// information functions
void PPPFrame::info(char *buf)
{
    cPacket::info( buf );
    sprintf( buf+strlen(buf), " Protocol: %x",
            _protocol);
}

void PPPFrame::writeContents(ostream& os)
{
    os << "PPPFrame: "
        << "\nProtocol " << (int)_protocol
        << "\n";
}

/* encapsulate a packet of type cPacket of the Network Layer;
    protocol set by default to IP;
    assumes that networkPacket->length() is
    length of transport packet in bits
    adds to it the PPP header length in bits */
//void PPPFrame::encapsulate(cPacket *networkPacket)
//{
//    cPacket::encapsulate(networkPacket);
//
// FIXME add this somewhere in the code:   _protocol = PPP_PROT_IP;
//}


