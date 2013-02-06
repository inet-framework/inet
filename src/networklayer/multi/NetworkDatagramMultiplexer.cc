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

#include "NetworkDatagramMultiplexer.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "IPv6ControlInfo.h"
#include "IPv6Datagram.h"
#include "GenericNetworkProtocolControlInfo.h"
#include "GenericDatagram.h"

Define_Module(NetworkDatagramMultiplexer);

void NetworkDatagramMultiplexer::handleMessage(cMessage * message) {
    cGate * arrivalGate = message->getArrivalGate();
    const char * arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "upperIn"))
        send(message, "lowerOut", getProtocolIndex(message));
    else if (!strcmp(arrivalGateName, "lowerIn"))
        send(message, "upperOut");
    else
        throw cRuntimeError("Unknown arrival gate");
}

int NetworkDatagramMultiplexer::getProtocolIndex(cMessage * message) {
    cPacket * packet = check_and_cast<cPacket *>(message);
    cObject * controlInfo = packet->getControlInfo();
    // TODO: handle the case when some network protocols are disabled
    if (dynamic_cast<IPv4ControlInfo *>(controlInfo) || dynamic_cast<IPv4Datagram *>(message))
        return 0;
    else if (dynamic_cast<IPv6ControlInfo *>(controlInfo) || dynamic_cast<IPv6Datagram *>(message))
        return 1;
    else if (dynamic_cast<GenericNetworkProtocolControlInfo *>(controlInfo) || dynamic_cast<GenericDatagram *>(message))
        return 2;
    else
        throw cRuntimeError("Unknown control info");
}
