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

#ifndef __INET_MPLS_H
#define __INET_MPLS_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/mpls/MPLSPacket.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/mpls/ConstType.h"

#include "inet/networklayer/mpls/LIBTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

#include "inet/networklayer/mpls/IClassifier.h"

namespace inet {

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
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

  protected:
    virtual void processPacketFromL3(cMessage *msg);
    virtual void processPacketFromL2(cMessage *msg);
    virtual void processMPLSPacketFromL2(MPLSPacket *mplsPacket);

    virtual bool tryLabelAndForwardIPv4Datagram(IPv4Datagram *ipdatagram);
    virtual void labelAndForwardIPv4Datagram(IPv4Datagram *ipdatagram);

    virtual void sendToL2(cMessage *msg, int gateIndex);
    virtual void doStackOps(MPLSPacket *mplsPacket, const LabelOpVector& outLabel);
};

} // namespace inet

#endif // ifndef __INET_MPLS_H

