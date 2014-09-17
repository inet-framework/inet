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
#include <math.h>

#include "inet/applications/ethernet/EtherTrafGen.h"

#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(EtherTrafGen);

simsignal_t EtherTrafGen::sentPkSignal = registerSignal("sentPk");
simsignal_t EtherTrafGen::rcvdPkSignal = registerSignal("rcvdPk");

EtherTrafGen::EtherTrafGen()
{
    sendInterval = NULL;
    numPacketsPerBurst = NULL;
    packetLength = NULL;
    timerMsg = NULL;
    nodeStatus = NULL;
}

EtherTrafGen::~EtherTrafGen()
{
    cancelAndDelete(timerMsg);
}

void EtherTrafGen::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        sendInterval = &par("sendInterval");
        numPacketsPerBurst = &par("numPacketsPerBurst");
        packetLength = &par("packetLength");
        etherType = par("etherType");

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        WATCH(packetsSent);
        WATCH(packetsReceived);

        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        if (isGenerator())
            timerMsg = new cMessage("generateNextPacket");

        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        if (isNodeUp() && isGenerator())
            scheduleNextPacket(-1);
    }
}

void EtherTrafGen::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    if (msg->isSelfMessage()) {
        if (msg->getKind() == START) {
            destMACAddress = resolveDestMACAddress();
            // if no dest address given, nothing to do
            if (destMACAddress.isUnspecified())
                return;
        }
        sendBurstPackets();
        scheduleNextPacket(simTime());
    }
    else
        receivePacket(check_and_cast<cPacket *>(msg));
}

bool EtherTrafGen::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isGenerator())
            scheduleNextPacket(-1);
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            cancelNextPacket();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            cancelNextPacket();
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

bool EtherTrafGen::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool EtherTrafGen::isGenerator()
{
    return par("destAddress").stringValue()[0];
}

void EtherTrafGen::scheduleNextPacket(simtime_t previous)
{
    simtime_t next;
    if (previous == -1) {
        next = simTime() <= startTime ? startTime : simTime();
        timerMsg->setKind(START);
    }
    else {
        next = previous + sendInterval->doubleValue();
        timerMsg->setKind(NEXT);
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timerMsg);
}

void EtherTrafGen::cancelNextPacket()
{
    cancelEvent(timerMsg);
}

MACAddress EtherTrafGen::resolveDestMACAddress()
{
    MACAddress destMACAddress;
    const char *destAddress = par("destAddress");
    if (destAddress[0]) {
        // try as mac address first, then as a module
        if (!destMACAddress.tryParse(destAddress)) {
            cModule *destStation = simulation.getModuleByPath(destAddress);
            if (!destStation)
                throw cRuntimeError("cannot resolve MAC address '%s': not a 12-hex-digit MAC address or a valid module path name", destAddress);

            cModule *destMAC = destStation->getSubmodule("mac");
            if (!destMAC)
                throw cRuntimeError("module '%s' has no 'mac' submodule", destAddress);

            destMACAddress.setAddress(destMAC->par("address"));
        }
    }
    return destMACAddress;
}

void EtherTrafGen::sendBurstPackets()
{
    int n = numPacketsPerBurst->longValue();
    for (int i = 0; i < n; i++) {
        seqNum++;

        char msgname[40];
        sprintf(msgname, "pk-%d-%ld", getId(), seqNum);

        cPacket *datapacket = new cPacket(msgname, IEEE802CTRL_DATA);

        long len = packetLength->longValue();
        datapacket->setByteLength(len);

        Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
        etherctrl->setEtherType(etherType);
        etherctrl->setDest(destMACAddress);
        datapacket->setControlInfo(etherctrl);

        EV_INFO << "Send packet `" << msgname << "' dest=" << destMACAddress << " length=" << len << "B type=" << etherType << "\n";
        emit(sentPkSignal, datapacket);
        send(datapacket, "out");
        packetsSent++;
    }
}

void EtherTrafGen::receivePacket(cPacket *msg)
{
    EV_INFO << "Received packet `" << msg->getName() << "' length= " << msg->getByteLength() << "B\n";

    packetsReceived++;
    emit(rcvdPkSignal, msg);
    delete msg;
}

void EtherTrafGen::finish()
{
    cancelAndDelete(timerMsg);
    timerMsg = NULL;
}

} // namespace inet

