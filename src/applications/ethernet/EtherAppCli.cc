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

#include "EtherAppCli.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"
#include "NodeOperations.h"
#include "ModuleAccess.h"

Define_Module(EtherAppCli);

simsignal_t EtherAppCli::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t EtherAppCli::rcvdPkSignal = SIMSIGNAL_NULL;

EtherAppCli::EtherAppCli()
{
    reqLength = NULL;
    respLength = NULL;
    sendInterval = NULL;
    timerMsg = NULL;
    nodeStatus = NULL;
}

EtherAppCli::~EtherAppCli()
{
    cancelAndDelete(timerMsg);
}

void EtherAppCli::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage==1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage == 1)
    {
        reqLength = &par("reqLength");
        respLength = &par("respLength");
        sendInterval = &par("sendInterval");

        localSAP = ETHERAPP_CLI_SAP;
        remoteSAP = ETHERAPP_SRV_SAP;

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        sentPkSignal = registerSignal("sentPk");
        rcvdPkSignal = registerSignal("rcvdPk");
        WATCH(packetsSent);
        WATCH(packetsReceived);

        destMACAddress = resolveDestMACAddress();

        // if no dest address given, nothing to do
        if (destMACAddress.isUnspecified())
            return;

        bool registerSAP = par("registerSAP");
        if (registerSAP)
            registerDSAP(localSAP);

        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime != -1 && stopTime < startTime)
            error("Invalid startTime/stopTime parameters");

        timerMsg = new cMessage("generateNextPacket");
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));

        if (isNodeUp() && isEnabled())
            scheduleNextPacket(-1);
    }
}

void EtherAppCli::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    if (msg->isSelfMessage())
    {
        sendPacket();
        scheduleNextPacket(simTime());
    }
    else
        receivePacket(check_and_cast<cPacket*>(msg));
}

bool EtherAppCli::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER && isEnabled())
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
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

bool EtherAppCli::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool EtherAppCli::isEnabled()
{
    return par("destAddress").stringValue()[0];
}

void EtherAppCli::scheduleNextPacket(simtime_t previous)
{
    simtime_t next;
    if (previous == -1)
        next = simTime() <= startTime ? startTime : simTime();
    else
        next = previous + sendInterval->doubleValue();
    if (stopTime == -1  || next <= stopTime)
        scheduleAt(next, timerMsg);
}

void EtherAppCli::cancelNextPacket()
{
    cancelEvent(timerMsg);
}

MACAddress EtherAppCli::resolveDestMACAddress()
{
    MACAddress destMACAddress;
    const char *destAddress = par("destAddress");
    if (destAddress[0])
    {
        // try as mac address first, then as a module
        if (!destMACAddress.tryParse(destAddress))
        {
            cModule *destStation = simulation.getModuleByPath(destAddress);
            if (!destStation)
                error("cannot resolve MAC address '%s': not a 12-hex-digit MAC address or a valid module path name", destAddress);

            cModule *destMAC = destStation->getSubmodule("mac");
            if (!destMAC)
                error("module '%s' has no 'mac' submodule", destAddress);

            destMACAddress.setAddress(destMAC->par("address"));
        }
    }
    return destMACAddress;
}

void EtherAppCli::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

void EtherAppCli::sendPacket()
{
    seqNum++;

    char msgname[30];
    sprintf(msgname, "req-%d-%ld", getId(), seqNum);
    EV << "Generating packet `" << msgname << "'\n";

    EtherAppReq *datapacket = new EtherAppReq(msgname, IEEE802CTRL_DATA);

    datapacket->setRequestId(seqNum);

    long len = reqLength->longValue();
    datapacket->setByteLength(len);

    long respLen = respLength->longValue();
    datapacket->setResponseBytes(respLen);

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    etherctrl->setDest(destMACAddress);
    datapacket->setControlInfo(etherctrl);

    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void EtherAppCli::receivePacket(cPacket *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";

    packetsReceived++;
    emit(rcvdPkSignal, msg);
    delete msg;
}

void EtherAppCli::finish()
{
    cancelAndDelete(timerMsg);
    timerMsg = NULL;
}

