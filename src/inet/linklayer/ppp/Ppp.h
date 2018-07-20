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

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/base/MacBase.h"

namespace inet {

class InterfaceEntry;
class IPassiveQueue;

/**
 * PPP implementation.
 */
class INET_API Ppp : public MacBase
{
  protected:
    bool sendRawBytes = false;
    long txQueueLimit = -1;
    cGate *physOutGate = nullptr;
    cChannel *datarateChannel = nullptr;    // nullptr if we're not connected

    cQueue txQueue;
    cMessage *endTransmissionEvent = nullptr;
    IPassiveQueue *queueModule = nullptr;

    std::string oldConnColor;

    // statistics
    long numSent = 0;
    long numRcvdOK = 0;
    long numBitErr = 0;
    long numDroppedIfaceDown = 0;

    static simsignal_t transmissionStateChangedSignal;
    static simsignal_t rxPkOkSignal;

  protected:
    virtual void startTransmitting(Packet *msg);
    virtual Packet *encapsulate(Packet *msg);
    virtual cPacket *decapsulate(Packet *packet);
    virtual void refreshDisplay() const override;
    virtual void refreshOutGateConnection(bool connected);

    // cListener function
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // MacBase functions
    virtual InterfaceEntry *createInterfaceEntry() override;
    virtual bool isUpperMsg(cMessage *msg) override { return msg->arrivedOn("upperLayerIn"); }
    virtual void flushQueue() override;
    virtual void clearQueue() override;

  public:
    virtual ~Ppp();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif // ifndef __INET_PPP_H

