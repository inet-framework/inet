//
// Copyright (C) 2010 Alfonso Ariza Quintana
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


#include "Ieee80211MgmtAdhocWithEtx.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Ieee80211MgmtAdhocWithEtx);


void Ieee80211MgmtAdhocWithEtx::initialize(int stage)
{
    Ieee80211MgmtAdhoc::initialize(stage);
    if (stage==1)
    {
        ETXProcess = NULL;
        if (par("ETXEstimate"))
        {
            cModuleType *moduleType;
            cModule *module;
            moduleType = cModuleType::find("inet.linklayer.ieee80211.mgmt.Ieee80211Etx");
            module = moduleType->create("ETXproc", this);
            ETXProcess = dynamic_cast <Ieee80211Etx*> (module);
            ETXProcess->gate("toMac")->connectTo(gate("ETXProcIn"));
            gate("ETXProcOut")->connectTo(ETXProcess->gate("fromMac"));
            ETXProcess->buildInside();
            ETXProcess->scheduleStart(simTime());
            ETXProcess->setAddress(myAddress);
        }
    }
}


void Ieee80211MgmtAdhocWithEtx::handleMessage(cMessage *msg)
{

    cGate * msggate = msg->getArrivalGate();
    char gateName [40];
    memset(gateName, 0, 40);
    strcpy(gateName, msggate->getBaseName());
    if (strstr(gateName, "ETXProcIn")!=NULL)
    {
        handleEtxMessage(PK(msg));
    }
    else
    {
        Ieee80211MgmtAdhoc::handleMessage(msg);

    }
}

void Ieee80211MgmtAdhocWithEtx::handleDataFrame(Ieee80211DataFrame *frame)
{
    ///
    /// If it's a ETX packet to send to the appropriate module
    ///
    if (dynamic_cast<ETXBasePacket*>(frame->getEncapsulatedPacket()))
    {
        if (ETXProcess)
        {
            cPacket *msg = decapsulate(frame);
            if (msg->getControlInfo())
                delete msg->removeControlInfo();
            send(msg, "ETXProcOut");
        }
        else
            delete frame;
        return;
    }
    else
        Ieee80211MgmtAdhoc::handleDataFrame(frame);
}

void Ieee80211MgmtAdhocWithEtx::handleEtxMessage(cPacket *pk)
{
    ETXBasePacket * etxMsg = dynamic_cast<ETXBasePacket*>(pk);
    if (etxMsg)
    {
        Ieee80211DataFrame *frame = new Ieee80211DataFrameWithSNAP(etxMsg->getName());
        frame->setReceiverAddress(etxMsg->getDest());
        //TODO frame->setEtherType(...);
        frame->encapsulate(etxMsg);
        sendOrEnqueue(frame);
    }
    else
        delete pk;
}

