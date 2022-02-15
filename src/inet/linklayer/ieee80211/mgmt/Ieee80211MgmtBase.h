//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MGMTBASE_H
#define __INET_IEEE80211MGMTBASE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

namespace ieee80211 {

/**
 * Abstract base class for 802.11 infrastructure mode management components.
 *
 */
class INET_API Ieee80211MgmtBase : public OperationalBase
{
  protected:
    // configuration
    ModuleRefByPar<Ieee80211Mib> mib;
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    NetworkInterface *myIface = nullptr;

    // statistics
    long numMgmtFramesReceived;
    long numMgmtFramesDropped;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;

    /** Dispatches incoming messages to handleTimer(), handleUpperMessage() or processFrame(). */
    virtual void handleMessageWhenUp(cMessage *msg) override;

    /** Should be redefined to deal with self-messages */
    virtual void handleTimer(cMessage *frame) = 0;

    /** Should be redefined to handle commands from the "agent" (if present) */
    virtual void handleCommand(int msgkind, cObject *ctrl) = 0;

    /** Utility method for implementing handleUpperMessage(): send message to MAC */
    virtual void sendDown(Packet *frame);

    /** Utility method to dispose of an unhandled frame */
    virtual void dropManagementFrame(Packet *frame);

    /** Dispatch to frame processing methods according to frame type */
    virtual void processFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header);

    /** @name Processing of different frame types */
    //@{
    virtual void handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    virtual void handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) = 0;
    //@}

    /** lifecycle support */
    //@{
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION; } // TODO INITSTAGE
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_PHYSICAL_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_PHYSICAL_LAYER; }

    virtual void handleStartOperation(LifecycleOperation *operation) override { start(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { stop(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { stop(); }

  protected:
    virtual void start();
    virtual void stop();
    //@}
};

} // namespace ieee80211

} // namespace inet

#endif

