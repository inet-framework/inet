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

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omnetpp.h>

#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "_802Ctrl_m.h"
#include "utils.h"



/**
 * Simple traffic generator to test the Ethernet models.
 */
class EtherTrafGen : public cSimpleModule
{
    double waitMean;
    int protocolId;
    int seqNum;
    cPar *dataLength;
    MACAddress destAddr;

  public:
    Module_Class_Members(EtherTrafGen,cSimpleModule,0);

    virtual void initialize(int stage);
    virtual int numInitStages() const {return 2;}
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    void sendPacket();
    void registerDSAP();
};

Define_Module (EtherTrafGen);

void EtherTrafGen::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage 1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage!=1) return;

    dataLength = &par("dataLength"); // bytes
    waitMean = par("waitMean");

    const char *destAddrStr = par("destAddress");
    const char *destStation = par("destStation");
    if (strcmp(destAddrStr,"") && strcmp(destStation,""))
    {
        error("only one of the `destAddress' and `destStation' module parameters should be filled in");
    }
    else if (strcmp(destAddrStr,""))
    {
        destAddr.setAddress(destAddrStr);
    }
    else if (strcmp(destStation,""))
    {
        std::string destModName = std::string(destStation) + ".mac";
        cModule *destMod = simulation.moduleByPath(destModName.c_str());
        if (!destMod)
            error("module `%s' (MAC submodule of `destStation') not found", destModName.c_str());
        destAddr.setAddress(destMod->par("address"));
    }

    // if no dest address given, nothing to do
    if (destAddr.isEmpty())
        return;

    seqNum = 0;
    WATCH(seqNum);

    cMessage *timermsg = new cMessage("generateNextPacket");
    scheduleAt(exponential(waitMean), timermsg);
}

void EtherTrafGen::handleMessage(cMessage *timermsg)
{
    sendPacket();
    scheduleAt(simTime()+exponential(waitMean), timermsg);
}

void EtherTrafGen::sendPacket()
{
    char msgname[30];
    sprintf(msgname, "data-%d-%d", id(), seqNum++);

    cMessage *datapacket = new cMessage(msgname, ETHCTRL_DATA);

    _802Ctrl *etherctrl = new _802Ctrl();
    etherctrl->setDest(destAddr);
    etherctrl->setSsap(protocolId);
    etherctrl->setDsap(protocolId);
    datapacket->setControlInfo(etherctrl);

    long len = 8*dataLength->longValue();
    datapacket->setLength(len);

    EV << "Generating packet `" << msgname << "'\n";

    send(datapacket, "out");
}

void EtherTrafGen::finish ()
{
}

