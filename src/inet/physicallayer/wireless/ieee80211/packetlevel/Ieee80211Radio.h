//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RADIO_H
#define __INET_IEEE80211RADIO_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatRadioBase.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetCoordinator.h"
#include "inet/physicallayer/wireless/ieee80211/contract/IIeee80211CcaProvider.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Receiver.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211Radio : public FlatRadioBase, public IIeee80211Radio, public IIeee80211CcaProvider
{
  public:
    /**
     * This signal is emitted during physical-layer initialization when a channel
     * is configured, and every time the radio channel or band changes. The value
     * is the band-local channel index; Ieee80211RadioChannelChangedDetails carries
     * the immutable band reference.
     */
    static simsignal_t radioChannelChangedSignal;
    static const Ptr<const Ieee80211PhyHeader> popIeee80211PhyHeaderAtFront(Packet *packet, b length = b(-1), int flags = 0);
    static const Ptr<const Ieee80211PhyHeader> peekIeee80211PhyHeaderAtFront(const Packet *packet, b length = b(-1), int flags = 0);

  protected:
    ModuleRefByPar<IIeee80211ModeSetCoordinator> modeSetCoordinator;
    bool changingModeSet = false;
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    Ieee80211SecondaryChannelOffset htSecondaryChannelOffset = IEEE80211_SECONDARY_CHANNEL_NONE;
    std::unique_ptr<Ieee80211CcaSnapshot> ccaSnapshot;
    std::string opMode;
    const Ieee80211ModeSet *modeSet = nullptr;
    const IIeee80211Band *band = nullptr;

  protected:
    virtual void initialize(int stage) override;

    void changeModeSet(const Ieee80211ModeSet *modeSet, const IIeee80211Mode *mode, bool explicitMode,
            const std::function<void()>& applyConfiguration = {}, bool publishModeSet = true, int channelNumber = -1);
    void abortIncompatibleReception();

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

    // Update behavioral consumers before publishing the new mode set.
    // Failures are fatal simulation errors; these setters do not roll back.
    // Behavioral consumers implement IIeee80211ModeSetListener.
    virtual const Ieee80211Channel *getChannel() const override;
    virtual bool isHtChannelWidthSupported(Hz channelWidth) const override;
    virtual void setModeSet(const Ieee80211ModeSet *modeSet);
    virtual void setModeSetAndMode(const Ieee80211ModeSet *modeSet, const IIeee80211Mode *mode);
    virtual void setMode(const IIeee80211Mode *mode);
    virtual void setBand(const IIeee80211Band *band);
    virtual void setChannel(const Ieee80211Channel *channel);
    virtual void setChannelNumber(int newChannelNumber);
};

} // namespace physicallayer
} // namespace inet

#endif
