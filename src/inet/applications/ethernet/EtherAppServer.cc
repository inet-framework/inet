//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/ethernet/EtherAppServer.h"

#include "inet/applications/ethernet/EtherApp_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
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

void EtherAppServer::socketDataArrived(Ieee8022LlcSocket *, Packet *msg)
{
    EV_INFO << "Received packet `" << msg->getName() << "'\n";
    const auto& req = msg->peekAtFront<EtherAppReq>();
    if (req == nullptr)
        throw cRuntimeError("data type error: not an EtherAppReq arrived in packet %s", msg->str().c_str());
    packetsReceived++;
    emit(packetReceivedSignal, msg);

    MacAddress srcAddr = msg->getTag<MacAddressInd>()->getSrcAddress();
    int srcInterfaceId = msg->getTag<InterfaceInd>()->getInterfaceId();
    int srcSap = msg->getTag<Ieee802SapInd>()->getSsap();
    long requestId = req->getRequestId();
    long replyBytes = req->getResponseBytes();

    // send back packets asked by EtherAppClient side
    for (int k = 0; replyBytes > 0; k++) {
        int l = replyBytes > MAX_REPLY_CHUNK_SIZE ? MAX_REPLY_CHUNK_SIZE : replyBytes;
        replyBytes -= l;

        std::ostringstream s;
        s << msg->getName() << "-resp-" << k;

        Packet *outPacket = new Packet(s.str().c_str(), IEEE802CTRL_DATA);
        const auto& outPayload = makeShared<EtherAppResp>();
        outPayload->setRequestId(requestId);
        outPayload->setChunkLength(B(l));
        outPayload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        outPacket->insertAtBack(outPayload);

        EV_INFO << "Send response `" << outPacket->getName() << "' to " << srcAddr << " ssap=" << localSap << " dsap=" << srcSap << " length=" << l << "B requestId=" << requestId << "\n";

        sendPacket(outPacket, srcInterfaceId, srcAddr, srcSap);
    }

    delete msg;
}

void EtherAppServer::sendPacket(Packet *datapacket, int interfaceId, const MacAddress& destAddr, int destSap)
{
    datapacket->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
    datapacket->addTag<MacAddressReq>()->setDestAddress(destAddr);
    auto ieee802SapReq = datapacket->addTag<Ieee802SapReq>();
    ieee802SapReq->setSsap(localSap);
    ieee802SapReq->setDsap(destSap);

    emit(packetSentSignal, datapacket);
    llcSocket.send(datapacket);
    packetsSent++;
}

void EtherAppServer::registerDsap(int dsap)
{
    EV_DEBUG << getFullPath() << " registering DSAP " << dsap << "\n";

    llcSocket.open(-1, dsap, -1);
}

void EtherAppServer::finish()
{
}

} // namespace inet

