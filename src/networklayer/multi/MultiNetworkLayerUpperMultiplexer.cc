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

#include "MultiNetworkLayerUpperMultiplexer.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"
#include "GenericNetworkProtocolControlInfo.h"

Define_Module(MultiNetworkLayerUpperMultiplexer);

void MultiNetworkLayerUpperMultiplexer::handleMessage(cMessage * message)
{
    cGate * arrivalGate = message->getArrivalGate();
    const char * arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "transportUpperIn")) {
        if (dynamic_cast<cPacket *>(message))
            sendSync(message, "transportLowerOut", getProtocolCount() * arrivalGate->getIndex() + getProtocolIndex(message));
        else {
            // sending down commands
            for (int i = 0; i < 3; i++) {
                cMessage * duplicate = message->dup();
                duplicate->setControlInfo(message->getControlInfo()->dup());
                sendSync(duplicate, "transportLowerOut", getProtocolCount() * arrivalGate->getIndex() + i);
            }
            delete message;
        }
    }
    else if (!strcmp(arrivalGateName, "transportLowerIn"))
        sendSync(message, "transportUpperOut", arrivalGate->getIndex() / getProtocolCount());
    else if (!strcmp(arrivalGateName, "pingUpperIn"))
        sendSync(message, "pingLowerOut", getProtocolCount() * arrivalGate->getIndex() + getProtocolIndex(message));
    else if (!strcmp(arrivalGateName, "pingLowerIn"))
        sendSync(message, "pingUpperOut", arrivalGate->getIndex() / getProtocolCount());
    else
        throw cRuntimeError("Unknown arrival gate");
}

int MultiNetworkLayerUpperMultiplexer::getProtocolCount()
{
    return 3;
}

int MultiNetworkLayerUpperMultiplexer::getProtocolIndex(cMessage * message)
{
    cPacket * packet = check_and_cast<cPacket *>(message);
    cObject * controlInfo = packet->getControlInfo();
    // TODO: handle the case when some network protocols are disabled
    if (dynamic_cast<IPv4ControlInfo *>(controlInfo))
        return 0;
    else if (dynamic_cast<IPv6ControlInfo *>(controlInfo))
        return 1;
    else if (dynamic_cast<GenericNetworkProtocolControlInfo *>(controlInfo))
        return 2;
    else
        throw cRuntimeError("Unknown control info");
}
