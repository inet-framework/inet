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
#include "Ieee802Ctrl_m.h"
#include "EtherApp_m.h"


Define_Module (EtherAppCli);

void EtherAppCli::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage==1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage == 1)
    {
        reqLength = &par("reqLength");
        respLength = &par("respLength");
        waitTime = &par("waitTime");

        localSAP = ETHERAPP_CLI_SAP;
        remoteSAP = ETHERAPP_SRV_SAP;

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        eedVector.setName("end-to-end delay");
        eedStats.setName("end-to-end delay");
        WATCH(packetsSent);
        WATCH(packetsReceived);

        destMACAddress = resolveDestMACAddress();

        // if no dest address given, nothing to do
        if (destMACAddress.isUnspecified())
            return;

        bool registerSAP = par("registerSAP");
        if (registerSAP)
            registerDSAP(localSAP);

        cMessage *timermsg = new cMessage("generateNextPacket");
        simtime_t d = par("startTime").doubleValue();
        scheduleAt(simTime()+d, timermsg);
    }
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

void EtherAppCli::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        sendPacket();
        simtime_t d = waitTime->doubleValue();
        scheduleAt(simTime()+d, msg);
    }
    else
    {
        receivePacket(msg);
    }
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

    send(datapacket, "out");
    packetsSent++;
}

void EtherAppCli::receivePacket(cMessage *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";

    packetsReceived++;
    simtime_t lastEED = simTime() - msg->getCreationTime();
    eedVector.record(lastEED);
    eedStats.collect(lastEED);

    delete msg;
}

void EtherAppCli::finish()
{
    recordScalar("packets sent", packetsSent);
    recordScalar("packets rcvd", packetsReceived);
    recordScalar("end-to-end delay mean", eedStats.getMean());
    recordScalar("end-to-end delay stddev", eedStats.getStddev());
    recordScalar("end-to-end delay min", eedStats.getMin());
    recordScalar("end-to-end delay max", eedStats.getMax());
}

