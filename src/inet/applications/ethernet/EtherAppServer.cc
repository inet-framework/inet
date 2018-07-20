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
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"

namespace inet {

Define_Module(EtherAppServer);

void EtherAppServer::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localSAP = par("localSAP");

        // statistics
        packetsSent = packetsReceived = 0;

        WATCH(packetsSent);
        WATCH(packetsReceived);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));

        if (isNodeUp())
            startApp();
    }
}

bool EtherAppServer::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void EtherAppServer::startApp()
{
    EV_INFO << "Starting application\n";
    bool registerSAP = par("registerSAP");
    if (registerSAP)
        registerDSAP(localSAP);
}

void EtherAppServer::stopApp()
{
    EV_INFO << "Stop the application\n";
}

void EtherAppServer::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");

    EV_INFO << "Received packet `" << msg->getName() << "'\n";
    Packet *reqPk = check_and_cast<Packet *>(msg);
    const auto& req = reqPk->peekDataAt<EtherAppReq>(B(0));
    if (req == nullptr)
        throw cRuntimeError("data type error: not an EtherAppReq arrived in packet %s", reqPk->str().c_str());
    packetsReceived++;
    emit(packetReceivedSignal, reqPk);

    MacAddress srcAddr = reqPk->getTag<MacAddressInd>()->getSrcAddress();
    int srcSap = reqPk->getTag<Ieee802SapInd>()->getSsap();
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
        outPacket->insertAtBack(outPayload);

        EV_INFO << "Send response `" << outPacket->getName() << "' to " << srcAddr << " ssap=" << localSAP << " dsap=" << srcSap << " length=" << l << "B requestId=" << requestId << "\n";

        sendPacket(outPacket, srcAddr, srcSap);
    }

    delete msg;
}

void EtherAppServer::sendPacket(Packet *datapacket, const MacAddress& destAddr, int destSap)
{
    datapacket->addTagIfAbsent<MacAddressReq>()->setDestAddress(destAddr);
    auto ieee802SapReq = datapacket->addTagIfAbsent<Ieee802SapReq>();
    ieee802SapReq->setSsap(localSAP);
    ieee802SapReq->setDsap(destSap);

    emit(packetSentSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void EtherAppServer::registerDSAP(int dsap)
{
    EV_DEBUG << getFullPath() << " registering DSAP " << dsap << "\n";

    auto *etherctrl = new Ieee802RegisterDsapCommand();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

bool EtherAppServer::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_APPLICATION_LAYER)
            startApp();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            stopApp();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH)
            stopApp();
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void EtherAppServer::finish()
{
}

} // namespace inet

