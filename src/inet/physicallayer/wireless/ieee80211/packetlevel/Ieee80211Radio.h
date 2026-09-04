//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RADIO_H
#define __INET_IEEE80211RADIO_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatRadioBase.h"
#include "inet/physicallayer/wireless/ieee80211/contract/IIeee80211CcaProvider.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211Radio : public FlatRadioBase, public IIeee80211CcaProvider
{
  public:
    /**
     * This signal is emitted every time the radio channel changes.
     * The signal value is the new radio channel.
     */
    static simsignal_t radioChannelChangedSignal;
    static const Ptr<const Ieee80211PhyHeader> popIeee80211PhyHeaderAtFront(Packet *packet, b length = b(-1), int flags = 0);
    static const Ptr<const Ieee80211PhyHeader> peekIeee80211PhyHeaderAtFront(const Packet *packet, b length = b(-1), int flags = 0);

  protected:
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    Ieee80211SecondaryChannelOffset htSecondaryChannelOffset = IEEE80211_SECONDARY_CHANNEL_NONE;
    std::unique_ptr<Ieee80211CcaSnapshot> ccaSnapshot;
    std::string opMode;
    const Ieee80211ModeSet *modeSet = nullptr;
    const IIeee80211Band *band = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual void handleUpperCommand(cMessage *message) override;

    virtual void insertFcs(const Ptr<Ieee80211PhyHeader>& phyHeader) const;
    virtual bool verifyFcs(const Ptr<const Ieee80211PhyHeader>& phyHeader) const;

    virtual void encapsulate(Packet *packet) const override;
    virtual void decapsulate(Packet *packet) const override;

    virtual bool computeIsBandBusy(Hz centerFrequency) const;
    virtual void updateCcaState();
    virtual void updateTransceiverState() override;

  public:
    Ieee80211Radio();

    virtual const Ieee80211CcaSnapshot& getCcaSnapshot() const override { return *ccaSnapshot; }
    virtual const Ieee80211Channel *getChannel() const { return check_and_cast<const Ieee80211Receiver *>(receiver)->getChannel(); }

    virtual void setModeSet(const Ieee80211ModeSet *modeSet);
    virtual void setMode(const IIeee80211Mode *mode);
    virtual void setBand(const IIeee80211Band *band);
    virtual void setChannel(const Ieee80211Channel *channel);
    virtual void setChannelNumber(int newChannelNumber);
};

} // namespace physicallayer
} // namespace inet

#endif

