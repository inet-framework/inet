//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __PPP_H
#define __PPP_H


#include <omnetpp.h>
#include "INETDefs.h"
#include "PPPFrame_m.h"
#include "TxNotifDetails.h"

class InterfaceEntry;
class IPassiveQueue;
class NotificationBoard;

/**
 * PPP implementation. Derived from the p-to-p OMNeT++ sample simulation.
 */
class INET_API PPP : public cSimpleModule
{
  protected:
    bool connected;

    long txQueueLimit;
    cGate *gateToWatch;

    cQueue txQueue;
    cMessage *endTransmissionEvent;
    IPassiveQueue *queueModule;

    InterfaceEntry *interfaceEntry;  // points into InterfaceTable
    double datarate;  // only cached for the display string

    NotificationBoard *nb;
    TxNotifDetails notifDetails;

    std::string oldConnColor;

    // statistics
    long numSent;
    long numRcvdOK;
    long numBitErr;
    long numDroppedIfaceDown;

  protected:
    InterfaceEntry *registerInterface(double datarate);
    void startTransmitting(cMessage *msg);
    PPPFrame *encapsulate(cMessage *msg);
    cMessage *decapsulate(PPPFrame *pppFrame);
    void displayBusy();
    void displayIdle();
    void updateDisplayString();

  public:
    PPP();
    virtual ~PPP();

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif


