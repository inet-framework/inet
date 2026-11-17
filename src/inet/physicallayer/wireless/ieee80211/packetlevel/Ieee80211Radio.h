//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RADIO_H
#define __INET_IEEE80211RADIO_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatRadioBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/IIeee80211ChannelAccess.h"
#include <array>
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211Radio : public FlatRadioBase, public IIeee80211ChannelAccess
{
  public:
    /**
     * This signal is emitted during physical-layer initialization when a channel
     * is configured, and every time the radio channel or band changes. The value
     * is the band-local channel index; Ieee80211RadioChannelChangedDetails carries
     * the immutable band reference.
     */
    static simsignal_t radioChannelChangedSignal;
    static simsignal_t primaryCcaChangedSignal;
    static const Ptr<const Ieee80211PhyHeader> popIeee80211PhyHeaderAtFront(Packet *packet, b length = b(-1), int flags = 0);
    static const Ptr<const Ieee80211PhyHeader> peekIeee80211PhyHeaderAtFront(const Packet *packet, b length = b(-1), int flags = 0);

  protected:
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    bool contiguousAllocation = false;
    bool primaryBusy = true;
    std::array<simtime_t, 3> secondaryIdleSince = { -1, -1, -1 };
    bool isBandBusy(Hz center, Hz width) const;

  protected:
    virtual void initialize(int stage) override;
    void updateTransceiverState() override;

    virtual void handleUpperCommand(cMessage *message) override;

    virtual void insertFcs(const Ptr<Ieee80211PhyHeader>& phyHeader) const;
    virtual bool verifyFcs(const Ptr<const Ieee80211PhyHeader>& phyHeader) const;

    virtual void encapsulate(Packet *packet) const override;
    virtual void decapsulate(Packet *packet) const override;

  public:
    Ieee80211Radio();

    virtual void setModeSet(const Ieee80211ModeSet *modeSet);
    void setRadioMode(RadioMode newRadioMode) override;
    virtual void setMode(const IIeee80211Mode *mode);
    virtual void setBand(const IIeee80211Band *band);
    virtual void setChannel(const Ieee80211Channel *channel);
    virtual void setChannelNumber(int newChannelNumber);
    void setBandwidth(Hz bandwidth) override;
    void setCenterFrequency(Hz centerFrequency) override;
    bool hasContiguousAllocation() const override { return contiguousAllocation; }
    const Ieee80211Channel *getOperatingChannel() const override;
    bool isPrimaryChannelBusy() const override { return (radioMode != RADIO_MODE_RECEIVER && radioMode != RADIO_MODE_TRANSCEIVER) || primaryBusy; }
    bool areSecondaryChannelsIdle(Hz width) const override;
};

} // namespace physicallayer
} // namespace inet

#endif
