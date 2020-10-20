//
// Copyright (C) 2013 OpenSim Ltd
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

#ifndef __INET_MACPROTOCOLBASE_H
#define __INET_MACPROTOCOLBASE_H

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

class INET_API MacProtocolBase : public LayeredProtocolBase, public cListener
{
  protected:
    /** @brief Gate ids */
    //@{
    int upperLayerInGateId = -1;
    int upperLayerOutGateId = -1;
    int lowerLayerInGateId = -1;
    int lowerLayerOutGateId = -1;
    //@}

    InterfaceEntry *interfaceEntry = nullptr;

    /** Currently transmitted frame if any */
    Packet *currentTxFrame = nullptr;

    /** Messages received from upper layer and to be transmitted later */
#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
    queueing::IPacketQueue *txQueue = nullptr;
#else
    opp_component_ptr<queueing::IPacketQueue> txQueue;
#endif

    cModule *hostModule = nullptr;

  protected:
    MacProtocolBase();
    virtual ~MacProtocolBase();

    virtual void initialize(int stage) override;

    virtual void registerInterface();
    virtual void configureInterfaceEntry() = 0;

    virtual MacAddress parseMacAddressParameter(const char *addrstr);

    virtual void sendUp(cMessage *message);
    virtual void sendDown(cMessage *message);

    virtual bool isUpperMessage(cMessage *message) override;
    virtual bool isLowerMessage(cMessage *message) override;

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }

    virtual void deleteCurrentTxFrame();
    virtual void dropCurrentTxFrame(PacketDropDetails& details);
    virtual void popTxQueue();

    /**
     * should clear queue and emit signal "packetDropped" with entire packets
     */
    virtual void flushQueue(PacketDropDetails& details);

    /**
     * should clear queue silently
     */
    virtual void clearQueue();

    using cListener::receiveSignal;
    virtual void handleMessageWhenDown(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif // ifndef __INET_MACPROTOCOLBASE_H

