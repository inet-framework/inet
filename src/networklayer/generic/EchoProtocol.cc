//
// Copyright (C) 2004, 2009 Andras Varga
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

#include <string.h>
#include "EchoProtocol.h"
#include "IPSocket.h"
#include "GenericNetworkProtocolControlInfo.h"
#include "PingPayload_m.h"

Define_Module(EchoProtocol);

void EchoProtocol::initialize(int stage)
{
    InetSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER_2)
    {
        IPSocket socket(gate("sendOut"));
        socket.registerProtocol(IP_PROT_ICMP);
    }
}

void EchoProtocol::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();
    if (!strcmp(arrivalGate->getName(), "localIn"))
        processPacket(check_and_cast<EchoPacket *>(msg));
    else if (!strcmp(arrivalGate->getName(), "pingIn"))
        sendEchoRequest(check_and_cast<PingPayload *>(msg));
}

void EchoProtocol::processPacket(EchoPacket *msg)
{
    switch (msg->getType())
    {
        case ECHO_PROTOCOL_REQUEST:
            processEchoRequest(msg);
            break;
        case ECHO_PROTOCOL_REPLY:
            processEchoReply(msg);
            break;
        default:
            throw cRuntimeError("Unknown type %d", msg->getType());
    }
}

void EchoProtocol::processEchoRequest(EchoPacket *request)
{
    // turn request into a reply
    EchoPacket *reply = request;
    reply->setName((std::string(request->getName()) + "-reply").c_str());
    reply->setType(ECHO_PROTOCOL_REPLY);
    // swap src and dest
    // TBD check what to do if dest was multicast etc?
    GenericNetworkProtocolControlInfo *ctrl = check_and_cast<GenericNetworkProtocolControlInfo *>(reply->getControlInfo());
    Address src = ctrl->getSourceAddress();
    Address dest = ctrl->getDestinationAddress();
    ctrl->setInterfaceId(-1);
    ctrl->setSourceAddress(dest);
    ctrl->setDestinationAddress(src);
    sendSync(reply, "sendOut");
}

void EchoProtocol::processEchoReply(EchoPacket *reply)
{
    GenericNetworkProtocolControlInfo *ctrl = check_and_cast<GenericNetworkProtocolControlInfo*>(reply->removeControlInfo());
    PingPayload *payload = check_and_cast<PingPayload *>(reply->decapsulate());
    payload->setControlInfo(ctrl);
    delete reply;
    long originatorId = payload->getOriginatorId();
    PingMap::iterator i = pingMap.find(originatorId);
    if (i != pingMap.end())
        sendSync(payload, "pingOut", i->second);
    else
    {
        EV << "Received ECHO REPLY has an unknown originator ID: " << originatorId << ", packet dropped." << endl;
        delete payload;
    }
}

void EchoProtocol::sendEchoRequest(PingPayload *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();
    int i = arrivalGate->getIndex();
    pingMap[msg->getOriginatorId()] = i;

    GenericNetworkProtocolControlInfo *ctrl = check_and_cast<GenericNetworkProtocolControlInfo*>(msg->removeControlInfo());
    ctrl->setProtocol(IP_PROT_ICMP);
    EchoPacket *request = new EchoPacket(msg->getName());
    request->setType(ECHO_PROTOCOL_REQUEST);
    request->encapsulate(msg);
    request->setControlInfo(ctrl);
    sendSync(request, "sendOut");
}
