//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_PPP_H
#define __INET_PPP_H


#include "INETDefs.h"

#include "PPPFrame_m.h"
#include "TxNotifDetails.h"
#include "INotifiable.h"
#include "ILifecycle.h"
#include "NodeStatus.h"
#include "MACBase.h"

class InterfaceEntry;
class IPassiveQueue;
class NotificationBoard;

/**
 * PPP implementation.
 */
class INET_API PPP : public MACBase, public cListener
{
  protected:
    long txQueueLimit;
    cGate *physOutGate;
    cChannel *datarateChannel; // NULL if we're not connected

    cQueue txQueue;
    cMessage *endTransmissionEvent;
    IPassiveQueue *queueModule;

    TxNotifDetails notifDetails;

    std::string oldConnColor;

    // statistics
    long numSent;
    long numRcvdOK;
    long numBitErr;
    long numDroppedIfaceDown;

    static simsignal_t txStateSignal;
    static simsignal_t rxPkOkSignal;
    static simsignal_t dropPkIfaceDownSignal;
    static simsignal_t dropPkBitErrorSignal;
    static simsignal_t packetSentToLowerSignal;
    static simsignal_t packetReceivedFromLowerSignal;
    static simsignal_t packetSentToUpperSignal;
    static simsignal_t packetReceivedFromUpperSignal;

  protected:
    virtual void startTransmitting(cPacket *msg);
    virtual PPPFrame *encapsulate(cPacket *msg);
    virtual cPacket *decapsulate(PPPFrame *pppFrame);
    virtual void displayBusy();
    virtual void displayIdle();
    virtual void updateDisplayString();
    virtual void refreshOutGateConnection(bool connected);

    // cListener function
    virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj);

    // MACBase functions
    virtual InterfaceEntry *createInterfaceEntry();
    virtual bool isUpperMsg(cMessage *msg) {return msg->arrivedOn("netwIn");}
    virtual void flushQueue();
    virtual void clearQueue();

  public:
    PPP();
    virtual ~PPP();

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif
