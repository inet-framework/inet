//
// Copyright (C) 2006 Levente Meszaros
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

#ifndef __INET_ETHER_DUPLEX_MAC_H
#define __INET_ETHER_DUPLEX_MAC_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "EtherFrame_m.h"
#include "EtherMACBase.h"

/**
 * A simplified version of EtherMAC. Since modern Ethernets typically
 * operate over duplex links where's no contention, the original CSMA/CD
 * algorithm is no longer needed. This simplified implementation doesn't
 * contain CSMA/CD, frames are just simply queued and sent out one by one.
 */
class INET_API EtherMAC2 : public EtherMACBase
{
  public:
    EtherMAC2();

  protected:
    virtual void initialize();
    virtual void initializeTxrate();
    virtual void handleMessage(cMessage *msg);

    // event handlers
    virtual void startFrameTransmission();
    virtual void processFrameFromUpperLayer(EtherFrame *frame);
    virtual void processMsgFromNetwork(cPacket *msg);
    virtual void handleEndIFGPeriod();
    virtual void handleEndTxPeriod();

    // notifications
    virtual void updateHasSubcribers();
};

#endif

