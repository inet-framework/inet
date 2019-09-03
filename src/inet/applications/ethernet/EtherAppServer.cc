/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include "inet/applications/ethernet/EtherAppServer.h"

#include "inet/applications/ethernet/EtherApp_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

namespace inet {

Define_Module(EtherAppServer);

void EtherAppServer::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localSap = par("localSAP");

        // statistics
        packetsSent = packetsReceived = 0;

        // socket
        llcSocket.setOutputGate(gate("out"));
        llcSocket.setCallback(this);

        WATCH(packetsSent);
        WATCH(packetsReceived);
    }
}

void EtherAppServer::handleStartOperation(LifecycleOperation *operation)
{
    EV_INFO << "Starting application\n";
    registerDsap(localSap);
}

void EtherAppServer::handleStopOperation(LifecycleOperation *operation)
{
    EV_INFO << "Stop the application\n";
    llcSocket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void EtherAppServer::handleCrashOperation(LifecycleOperation *operation)
{
    EV_INFO << "Crash the application\n";
    if (operation->getRootModule() != getContainingNode(this))
        llcSocket.destroy();
}

void EtherAppServer::socketClosed(Ieee8022LlcSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION && !llcSocket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void EtherAppServer::handleMessageWhenUp(cMessage *msg)
{
    llcSocket.processMessage(msg);
}

void EtherAppServer::socketDataArrived(Ieee8022LlcSocket*, Packet *msg)
{
    EV_INFO << "Received packet `" << msg->getName() << "'\n";
    const auto& req = msg->peekAtFront<EtherAppReq>();
    if (req == nullptr)
        throw cRuntimeError("data type error: not an EtherAppReq arrived in packet %s", msg->str().c_str());
    packetsReceived++;
    emit(packetReceivedSignal, msg);

    MacAddress srcAddr = msg->getTag<MacAddressInd>()->getSrcAddress();
    int srcSap = msg->getTag<Ieee802SapInd>()->getSsap();
    long requestId = req->getRequestId();
    long replyBytes = req->getResponseBytes();

    // send back packets asked by EtherAppClient side
    for (int k = 0; replyBytes > 0; k++) {
        int l = replyBytes > MAX_REPLY_CHUNK_SIZE ? MAX_REPLY_CHUNK_SIZE : replyBytes;
        replyBytes -= l;
        if (l < 12)
            l = 12;

        std::ostringstream s;
        s << msg->getName() << "-resp-" << k;

        Packet *outPacket = new Packet(s.str().c_str(), IEEE802CTRL_DATA);
        const auto& outPayload = makeShared<EtherAppResp>();
        outPayload->setRequestId(requestId);
        outPayload->setChunkLength(B(l));
        outPayload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        outPacket->insertAtBack(outPayload);

        EV_INFO << "Send response `" << outPacket->getName() << "' to " << srcAddr << " ssap=" << localSap << " dsap=" << srcSap << " length=" << l << "B requestId=" << requestId << "\n";

        sendPacket(outPacket, srcAddr, srcSap);
    }

    delete msg;
}

void EtherAppServer::sendPacket(Packet *datapacket, const MacAddress& destAddr, int destSap)
{
    datapacket->addTagIfAbsent<MacAddressReq>()->setDestAddress(destAddr);
    auto ieee802SapReq = datapacket->addTagIfAbsent<Ieee802SapReq>();
    ieee802SapReq->setSsap(localSap);
    ieee802SapReq->setDsap(destSap);

    emit(packetSentSignal, datapacket);
    llcSocket.send(datapacket);
    packetsSent++;
}

void EtherAppServer::registerDsap(int dsap)
{
    EV_DEBUG << getFullPath() << " registering DSAP " << dsap << "\n";

    llcSocket.open(-1, dsap);
}

void EtherAppServer::finish()
{
}

} // namespace inet

