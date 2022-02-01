//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MAC_H
#define __INET_IEEE80211MAC_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/ieee80211/mac/contract/IDs.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Mcf.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Pcf.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace ieee80211 {

class IContention;
class IRx;
class IIeee80211Llc;
class Ieee80211MacHeader;

/**
 * Implements the IEEE 802.11 MAC. The features, standards compliance and
 * exact operation of the MAC depend on the plugged-in components (see IUpperMac,
 * IRx, ITx, IContention and other interface classes).
 */
class INET_API Ieee80211Mac : public MacProtocolBase
{
  protected:
    FcsMode fcsMode;

    ModuleRefByPar<Ieee80211Mib> mib;
    opp_component_ptr<IIeee80211Llc> llc;
    opp_component_ptr<IDs> ds;

    opp_component_ptr<IRx> rx;
    opp_component_ptr<ITx> tx;
    opp_component_ptr<physicallayer::IRadio> radio;
    const physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_UNDEFINED;

    opp_component_ptr<Dcf> dcf;
    opp_component_ptr<Pcf> pcf;
    opp_component_ptr<Hcf> hcf;
    opp_component_ptr<Mcf> mcf;

    // The last change channel message received and not yet sent to the physical layer, or nullptr.
    cMessage *pendingRadioConfigMsg = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;
    virtual void initializeRadioMode();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;
    using MacProtocolBase::receiveSignal;
    virtual void configureRadioMode(physicallayer::IRadio::RadioMode radioMode);
    virtual void configureNetworkInterface() override;
    virtual const MacAddress& isInterfaceRegistered();

    virtual void handleMessageWhenUp(cMessage *message) override;

    /** @brief Handle commands (msg kind+control info) coming from upper layers */
    virtual void handleUpperCommand(cMessage *msg) override;

    /** @brief Handle timer self messages */
    virtual void handleSelfMessage(cMessage *msg) override;

    /** @brief Handle packets from management */
    virtual void handleMgmtPacket(Packet *packet);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(Packet *packet) override;

    /** @brief Handle messages from lower (physical) layer */
    virtual void handleLowerPacket(Packet *packet) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void encapsulate(Packet *packet);
    virtual void decapsulate(Packet *packet);

  public:
    Ieee80211Mac();
    virtual ~Ieee80211Mac();

    virtual FcsMode getFcsMode() const { return fcsMode; }
    virtual const MacAddress& getAddress() const { return mib->address; }
    virtual void sendUp(cMessage *message) override;
    virtual void sendUpFrame(Packet *frame);
    virtual void sendDownFrame(Packet *frame);
    virtual void sendDownPendingRadioConfigMsg();

    virtual void processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header);
    virtual void processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);
};

} // namespace ieee80211
} // namespace inet

#endif

