//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_MACRELAYUNIT_H
#define __INET_MACRELAYUNIT_H

#include "inet/common/INETDefs.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/switch/IMacAddressTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API MacRelayUnit : public LayeredProtocolBase
{
  protected:
    IInterfaceTable *ifTable = nullptr;
    IMacAddressTable *macTable = nullptr;

    // Parameters for statistics collection
    long numProcessedFrames = 0;
    long numDiscardedFrames = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void updateDisplayString() const;
    /**
     * Updates address table with source address, determines output port
     * and sends out (or broadcasts) frame on ports. Includes calls to
     * updateTableWithAddress() and getPortForAddress().
     *
     * The message pointer should not be referenced any more after this call.
     */
    virtual void handleAndDispatchFrame(Packet *packet);

    void handleLowerPacket(Packet *packet) override;

    /**
     * Utility function: sends the frame on all ports except inputport.
     * The message pointer should not be referenced any more after this call.
     */
    virtual void broadcast(Packet *frame, int inputport);

    /**
     * Writes statistics.
     */
    virtual void finish() override;

    //@{ for lifecycle:
    virtual void handleStartOperation(LifecycleOperation *operation) override { start(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { stop(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { stop(); }
    virtual bool isUpperMessage(cMessage *message) override { return message->arrivedOn("upperLayerIn"); }
    virtual bool isLowerMessage(cMessage *message) override { return message->arrivedOn("ifIn"); }

    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }

    virtual void start();
    virtual void stop();
    //@}

    virtual void learn(MacAddress srcAddr, int arrivalInterfaceId);
};

} // namespace inet

#endif // ifndef __INET_MACRELAYUNIT_H

