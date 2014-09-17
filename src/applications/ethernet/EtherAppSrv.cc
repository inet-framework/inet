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

#include "inet/applications/ethernet/EtherAppSrv.h"

#include "inet/applications/ethernet/EtherApp_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

Define_Module(EtherAppSrv);

simsignal_t EtherAppSrv::sentPkSignal = registerSignal("sentPk");
simsignal_t EtherAppSrv::rcvdPkSignal = registerSignal("rcvdPk");

void EtherAppSrv::initialize(int stage)
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

bool EtherAppSrv::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void EtherAppSrv::startApp()
{
    EV_INFO << "Starting application\n";
    bool registerSAP = par("registerSAP");
    if (registerSAP)
        registerDSAP(localSAP);
}

void EtherAppSrv::stopApp()
{
    EV_INFO << "Stop the application\n";
}

void EtherAppSrv::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");

    EV_INFO << "Received packet `" << msg->getName() << "'\n";
    EtherAppReq *req = check_and_cast<EtherAppReq *>(msg);
    packetsReceived++;
    emit(rcvdPkSignal, req);

    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
    MACAddress srcAddr = ctrl->getSrc();
    int srcSap = ctrl->getSsap();
    long requestId = req->getRequestId();
    long replyBytes = req->getResponseBytes();
    char msgname[30];
    strcpy(msgname, msg->getName());

    delete msg;
    delete ctrl;

    // send back packets asked by EtherAppCli side
    int k = 0;
    strcat(msgname, "-resp-");
    char *s = msgname + strlen(msgname);

    while (replyBytes > 0) {
        int l = replyBytes > MAX_REPLY_CHUNK_SIZE ? MAX_REPLY_CHUNK_SIZE : replyBytes;
        replyBytes -= l;

        sprintf(s, "%d", k);

        EtherAppResp *datapacket = new EtherAppResp(msgname, IEEE802CTRL_DATA);
        datapacket->setRequestId(requestId);
        datapacket->setByteLength(l);

        EV_INFO << "Send response `" << msgname << "' to " << srcAddr << " ssap=" << localSAP << " dsap=" << srcSap << " length=" << l << "B requestId=" << requestId << "\n";

        sendPacket(datapacket, srcAddr, srcSap);

        k++;
    }
}

void EtherAppSrv::sendPacket(cPacket *datapacket, const MACAddress& destAddr, int destSap)
{
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(destSap);
    etherctrl->setDest(destAddr);
    datapacket->setControlInfo(etherctrl);
    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void EtherAppSrv::registerDSAP(int dsap)
{
    EV_DEBUG << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

bool EtherAppSrv::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            startApp();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            stopApp();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            stopApp();
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void EtherAppSrv::finish()
{
}

} // namespace inet

