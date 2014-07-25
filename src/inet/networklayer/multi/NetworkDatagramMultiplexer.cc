//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
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

#include "inet/networklayer/multi/NetworkDatagramMultiplexer.h"

#ifdef WITH_IPv4
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_GENERIC
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo.h"
#include "inet/networklayer/generic/GenericDatagram.h"
#endif // ifdef WITH_GENERIC

namespace inet {

Define_Module(NetworkDatagramMultiplexer);

void NetworkDatagramMultiplexer::handleMessage(cMessage *message)
{
    cGate *arrivalGate = message->getArrivalGate();
    const char *arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "upperIn"))
        send(message, "lowerOut", getProtocolIndex(message));
    else if (!strcmp(arrivalGateName, "lowerIn"))
        send(message, "upperOut");
    else
        throw cRuntimeError("Unknown arrival gate");
}

int NetworkDatagramMultiplexer::getProtocolIndex(cMessage *message)
{
    cPacket *packet = check_and_cast<cPacket *>(message);
    cObject *controlInfo = packet->getControlInfo();
    // TODO: handle the case when some network protocols are disabled
    if (false)
        ;
#ifdef WITH_IPv4
    else if (dynamic_cast<IPv4ControlInfo *>(controlInfo) || dynamic_cast<IPv4Datagram *>(message))
        return 0;
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    else if (dynamic_cast<IPv6ControlInfo *>(controlInfo) || dynamic_cast<IPv6Datagram *>(message))
        return 1;
#endif // ifdef WITH_IPv6
#ifdef WITH_GENERIC
    else if (dynamic_cast<GenericNetworkProtocolControlInfo *>(controlInfo) || dynamic_cast<GenericDatagram *>(message))
        return 2;
#endif // ifdef WITH_GENERIC
    else
        throw cRuntimeError("Unknown control info");
}

} // namespace inet

