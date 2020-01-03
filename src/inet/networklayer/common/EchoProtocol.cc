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

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/EchoProtocol.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace inet {

Define_Module(EchoProtocol);

void EchoProtocol::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(Protocol::echo, nullptr, gate("ipIn"));
        registerProtocol(Protocol::echo, gate("ipOut"), nullptr);
    }
}

void EchoProtocol::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();
    if (!strcmp(arrivalGate->getName(), "ipIn"))
        processPacket(check_and_cast<Packet *>(msg));
}

void EchoProtocol::processPacket(Packet *msg)
{
    const auto& echoHdr = msg->peekAtFront<EchoPacket>();
    switch (echoHdr->getType()) {
        case ECHO_PROTOCOL_REQUEST:
            processEchoRequest(msg);
            break;

        case ECHO_PROTOCOL_REPLY:
            processEchoReply(msg);
            break;

        default:
            throw cRuntimeError("Unknown type %d", echoHdr->getType());
    }
}

void EchoProtocol::processEchoRequest(Packet *request)
{
    // turn request into a reply
    const auto& echoReq = request->popAtFront<EchoPacket>();
    Packet *reply = new Packet((std::string(request->getName()) + "-reply").c_str());
    const auto& echoReply = makeShared<EchoPacket>();
    echoReply->setChunkLength(echoReq->getChunkLength());
    echoReply->setType(ECHO_PROTOCOL_REPLY);
    echoReply->setIdentifier(echoReq->getIdentifier());
    echoReply->setSeqNumber(echoReq->getSeqNumber());
    reply->insertAtBack(echoReply);
    reply->insertAtBack(request->peekData());
    auto addressInd = request->getTag<L3AddressInd>();

    // swap src and dest
    // TBD check what to do if dest was multicast etc?
    auto addressReq = reply->addTag<L3AddressReq>();
    addressReq->setSrcAddress(addressInd->getDestAddress());
    addressReq->setDestAddress(addressInd->getSrcAddress());

    reply->addTag<DispatchProtocolReq>()->setProtocol(request->getTag<NetworkProtocolInd>()->getProtocol());
    reply->addTag<PacketProtocolTag>()->setProtocol(&Protocol::echo);
    send(reply, "ipOut");
    delete request;
}

void EchoProtocol::processEchoReply(Packet *reply)
{
    delete reply;
}

} // namespace inet

