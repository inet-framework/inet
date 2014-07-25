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

#include "inet/networklayer/multi/MultiNetworkLayerUpperMultiplexer.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo.h"

namespace inet {

Define_Module(MultiNetworkLayerUpperMultiplexer);

void MultiNetworkLayerUpperMultiplexer::initialize()
{
    int transportUpperInGateSize = gateSize("transportUpperIn");
    int transportUpperOutGateSize = gateSize("transportUpperOut");
    if (transportUpperInGateSize != transportUpperOutGateSize)
        throw cRuntimeError("Connection error: transportUpperIn[] and transportUpperOut[] gate size differ");
    int transportLowerInGateSize = gateSize("transportLowerIn");
    int transportLowerOutGateSize = gateSize("transportLowerOut");
    if (transportLowerInGateSize != transportLowerOutGateSize)
        throw cRuntimeError("Connection error: transportLowerIn[] and transportLowerOut[] gate size differ");
    if (transportUpperInGateSize * getProtocolCount() != transportLowerOutGateSize)
        throw cRuntimeError("Connection error: transportUpperIn[] / transportLowerOut[] gate size ratio not correct");
    int pingUpperInGateSize = gateSize("pingUpperIn");
    int pingUpperOutGateSize = gateSize("pingUpperOut");
    if (pingUpperInGateSize != pingUpperOutGateSize)
        throw cRuntimeError("Connection error: pingUpperIn[] and pingUpperOut[] gate size differ");
    int pingLowerInGateSize = gateSize("pingLowerIn");
    int pingLowerOutGateSize = gateSize("pingLowerOut");
    if (pingLowerInGateSize != pingLowerOutGateSize)
        throw cRuntimeError("Connection error: pingLowerIn[] and pingLowerOut[] gate size differ");
    if (pingUpperInGateSize * getProtocolCount() != pingLowerOutGateSize)
        throw cRuntimeError("Connection error: pingUpperIn[] / pingLowerOut[] gate size ratio not correct");
}

void MultiNetworkLayerUpperMultiplexer::handleMessage(cMessage *message)
{
    cGate *arrivalGate = message->getArrivalGate();
    const char *arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "transportUpperIn")) {
        if (dynamic_cast<cPacket *>(message))
            send(message, "transportLowerOut", getProtocolCount() * arrivalGate->getIndex() + getProtocolIndex(message));
        else {
            // sending down commands
            for (int i = 0; i < getProtocolCount(); i++) {
                cMessage *duplicate = message->dup();
                cObject *controlInfo = message->getControlInfo();
                if (controlInfo)
                    duplicate->setControlInfo(controlInfo->dup());
                send(duplicate, "transportLowerOut", getProtocolCount() * arrivalGate->getIndex() + i);
            }
            delete message;
        }
    }
    else if (!strcmp(arrivalGateName, "transportLowerIn"))
        send(message, "transportUpperOut", arrivalGate->getIndex() / getProtocolCount());
    else if (!strcmp(arrivalGateName, "pingUpperIn"))
        send(message, "pingLowerOut", getProtocolCount() * arrivalGate->getIndex() + getProtocolIndex(message));
    else if (!strcmp(arrivalGateName, "pingLowerIn"))
        send(message, "pingUpperOut", arrivalGate->getIndex() / getProtocolCount());
    else
        throw cRuntimeError("Unknown arrival gate");
}

int MultiNetworkLayerUpperMultiplexer::getProtocolCount()
{
    return 3;
}

int MultiNetworkLayerUpperMultiplexer::getProtocolIndex(cMessage *message)
{
    cPacket *packet = check_and_cast<cPacket *>(message);
    cObject *controlInfo = packet->getControlInfo();
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

} // namespace inet

