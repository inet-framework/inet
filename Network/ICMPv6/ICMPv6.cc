//
// Copyright (C) 2005 Andras Varga
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
//


#include <omnetpp.h>
#include "ICMPv6.h"


Define_Module(ICMPv6);


void ICMPv6::initialize()
{
    //...
}

void ICMPv6::handleMessage(cMessage *msg)
{
    //...
}

void ICMPv6::sendErrorMessage(IPv6Datagram *datagram, ICMPv6Type type, int code)
{
    //TBD ...
    error("sendErrorMessage(type=%d,code=%d) called", type, code);
}
