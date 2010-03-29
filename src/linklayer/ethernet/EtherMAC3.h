//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
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

#ifndef __INET_ETHER_MAC3_H
#define __INET_ETHER_MAC3_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "EtherFrame_m.h"
#include "EtherMAC2.h"

/**
 * Implementation of EtherMAC2 with frame loss handling at txQueue.
 */
class INET_API EtherMAC3 : public EtherMAC2
{
//  public:
//    EtherMAC3();

  protected:
	// statistics
	unsigned long numDroppedTxQueueOverflow; // packets from higher layer dropped due to txQueue overflow
	cOutVector numDroppedTxQueueOverflowVector;

  protected:
	// initialization
    virtual void initializeStatistics();
//	virtual void initialize();
//    virtual void initializeTxrate();
//    virtual void handleMessage(cMessage *msg);

    // finish
    virtual void finish();

//    // event handlers
//    virtual void startFrameTransmission();
    virtual void processFrameFromUpperLayer(EtherFrame *frame);
//    virtual void processMsgFromNetwork(cPacket *msg);
//    virtual void handleEndIFGPeriod();
//    virtual void handleEndTxPeriod();

//    // notifications
//    virtual void updateHasSubcribers();
};

#endif

