//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "MultiNetworkLayerLowerMultiplexer.h"

#ifdef WITH_IPv4
#include "ARPPacket_m.h"
#include "IPv4Datagram.h"
#endif

#ifdef WITH_IPv6
#include "IPv6Datagram.h"
#endif

#include "GenericDatagram.h"

Define_Module(MultiNetworkLayerLowerMultiplexer);

void MultiNetworkLayerLowerMultiplexer::handleMessage(cMessage * message)
{
    cGate * arrivalGate = message->getArrivalGate();
    const char * arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "ifUpperIn"))
        sendSync(message, "ifLowerOut", arrivalGate->getIndex() / getProtocolCount());
    else if (!strcmp(arrivalGateName, "ifLowerIn"))
        sendSync(message, "ifUpperOut", getProtocolCount() * arrivalGate->getIndex() + getProtocolIndex(message));
    else
        throw cRuntimeError("Unknown arrival gate");
}

int MultiNetworkLayerLowerMultiplexer::getProtocolCount()
{
    return 3;
}

int MultiNetworkLayerLowerMultiplexer::getProtocolIndex(cMessage * message)
{
    // TODO: handle the case when some network protocols are disabled
    if (false) ;
#ifdef WITH_IPv4
    if (dynamic_cast<IPv4Datagram *>(message) || dynamic_cast<ARPPacket *>(message))
        return 0;
#endif
#ifdef WITH_IPv6
    else if (dynamic_cast<IPv6Datagram *>(message))
        return 1;
#endif
    else if (dynamic_cast<GenericDatagram *>(message))
        return 2;
    else
        throw cRuntimeError("Unknown message");
}
