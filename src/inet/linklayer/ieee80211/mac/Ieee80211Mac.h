//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IEEE80211MAC_H
#define __INET_IEEE80211MAC_H

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
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

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

    Ieee80211Mib *mib = nullptr;
    IIeee80211Llc *llc = nullptr;
    IDs *ds = nullptr;

    IRx *rx = nullptr;
    ITx *tx = nullptr;
    physicallayer::IRadio *radio = nullptr;
    const physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    physicallayer::IRadio::TransmissionState transmissionState = physicallayer::IRadio::TransmissionState::TRANSMISSION_STATE_UNDEFINED;

    Dcf *dcf = nullptr;
    Pcf *pcf = nullptr;
    Hcf *hcf = nullptr;
    Mcf *mcf = nullptr;

    // The last change channel message received and not yet sent to the physical layer, or NULL.
    cMessage *pendingRadioConfigMsg = nullptr;

  protected:
    virtual int numInitStages() const override {return NUM_INIT_STAGES;}
    virtual void initialize(int) override;
    virtual void initializeRadioMode();

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;
    using MacProtocolBase::receiveSignal;
    virtual void configureRadioMode(physicallayer::IRadio::RadioMode radioMode);
    virtual void configureInterfaceEntry() override;
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

    void deleteCurrentTxFrame() override { throw cRuntimeError("model error"); }
    void dropCurrentTxFrame(PacketDropDetails& details) override { throw cRuntimeError("model error"); }
    void popTxQueue() override { throw cRuntimeError("model error"); }
};

} // namespace ieee80211
} // namespace inet

#endif
