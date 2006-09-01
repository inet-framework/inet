/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omnetpp.h>
#include "INETDefs.h"
#include "Ieee802Ctrl_m.h"
#include "EtherApp_m.h"
#include "utils.h"



/**
 * Simple traffic generator for the Ethernet model.
 */
class INET_API EtherAppCli : public cSimpleModule
{
    // send parameters
    long seqNum;
    cPar *reqLength;
    cPar *respLength;
    cPar *waitTime;

    int localSAP;
    int remoteSAP;
    MACAddress destMACAddress;

    // receive statistics
    long packetsSent;
    long packetsReceived;
    cOutVector eedVector;
    cStdDev eedStats;

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 2;}
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    MACAddress resolveDestMACAddress();

    void sendPacket();
    void receivePacket(cMessage *msg);
    void registerDSAP(int dsap);
};

Define_Module (EtherAppCli);

void EtherAppCli::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage 1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage!=1) return;

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
    double d = waitTime->doubleValue();
    scheduleAt(simTime()+d, timermsg);

}


MACAddress EtherAppCli::resolveDestMACAddress()
{
    MACAddress destMACAddress;
    const char *destAddr = par("destAddress");
    const char *destStation = par("destStation");
    if (strcmp(destAddr,"") && strcmp(destStation,""))
    {
        error("only one of the `destAddress' and `destStation' module parameters should be filled in");
    }
    else if (strcmp(destAddr,""))
    {
        destMACAddress.setAddress(destAddr);
    }
    else if (strcmp(destStation,""))
    {
        std::string destModName = std::string(destStation) + ".mac";
        cModule *destMod = simulation.moduleByPath(destModName.c_str());
        if (!destMod)
            error("module `%s' (MAC submodule of `destStation') not found", destModName.c_str());
        destMACAddress.setAddress(destMod->par("address"));
    }
    return destMACAddress;
}

void EtherAppCli::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        sendPacket();
        double d = waitTime->doubleValue();
        scheduleAt(simTime()+d, msg);
    }
    else
    {
        receivePacket(msg);
    }
}

void EtherAppCli::registerDSAP(int dsap)
{
    EV << fullPath() << " registering DSAP " << dsap << "\n";

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
    sprintf(msgname, "req-%d-%ld", id(), seqNum);
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
    EV << "Received packet `" << msg->name() << "'\n";

    packetsReceived++;
    simtime_t lastEED = simTime() - msg->creationTime();
    eedVector.record(lastEED);
    eedStats.collect(lastEED);

    delete msg;
}

void EtherAppCli::finish()
{
    if (par("writeScalars").boolValue())
    {
        recordScalar("packets sent", packetsSent);
        recordScalar("packets rcvd", packetsReceived);
        recordScalar("end-to-end delay mean", eedStats.mean());
        recordScalar("end-to-end delay stddev", eedStats.stddev());
        recordScalar("end-to-end delay min", eedStats.min());
        recordScalar("end-to-end delay max", eedStats.max());
    }
}

