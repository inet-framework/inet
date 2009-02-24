//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_MPLSMODULE_H
#define __INET_MPLSMODULE_H

#include <vector>
#include <omnetpp.h>

#include "MPLSPacket.h"
#include "IPDatagram.h"
#include "ConstType.h"

#include "LIBTable.h"
#include "IInterfaceTable.h"

#include "IClassifier.h"


/**
 * Implements the MPLS protocol; see the NED file for more info.
 */
class INET_API MPLS : public cSimpleModule
{
    protected:
        simtime_t delay1;

        //no longer used, see comment in intialize
        //std::vector<bool> labelIf;

        LIBTable *lt;
        IInterfaceTable *ift;
        IClassifier *pct;

    protected:
        virtual void initialize(int stage);
        virtual int numInitStages() const  {return 5;}
        virtual void handleMessage(cMessage *msg);

    protected:
        virtual void processPacketFromL3(cMessage *msg);
        virtual void processPacketFromL2(cMessage *msg);
        virtual void processMPLSPacketFromL2(MPLSPacket *mplsPacket);

        virtual bool tryLabelAndForwardIPDatagram(IPDatagram *ipdatagram);
        virtual void labelAndForwardIPDatagram(IPDatagram *ipdatagram);

        virtual void sendToL2(cMessage *msg, int gateIndex);
        virtual void doStackOps(MPLSPacket *mplsPacket, const LabelOpVector& outLabel);
};

#endif

