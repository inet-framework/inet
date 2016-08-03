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

#include "inet/common/IProtocolRegistrationListener.h"
#include <string.h>
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/EchoProtocol.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/applications/pingapp/PingPayload_m.h"

namespace inet {

Define_Module(EchoProtocol);

void EchoProtocol::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_NETWORK_LAYER)
        registerProtocol(Protocol::icmpv4, gate("ipOut"));
}

void EchoProtocol::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();
    if (!strcmp(arrivalGate->getName(), "ipIn"))
        processPacket(check_and_cast<EchoPacket *>(msg));
}

void EchoProtocol::processPacket(EchoPacket *msg)
{
    switch (msg->getType()) {
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
    INetworkProtocolControlInfo *ctrl = check_and_cast<INetworkProtocolControlInfo *>(reply->getControlInfo());
    auto addressInd = reply->removeMandatoryTag<L3AddressInd>();
    reply->clearTags();
    reply->ensureTag<DispatchProtocolReq>()->setProtocol(&Protocol::gnp);
    reply->ensureTag<ProtocolTag>()->setProtocol(&Protocol::icmpv4);
    auto addressReq = reply->ensureTag<L3AddressReq>();
    addressReq->setSource(addressInd->getDestination());
    addressReq->setDestination(addressInd->getSource());
    delete addressInd;
    send(reply, "ipOut");
}

void EchoProtocol::processEchoReply(EchoPacket *reply)
{
    delete reply;
}

} // namespace inet

