//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211RECEIVER_H
#define __INET_IEEE80211RECEIVER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211Receiver : public FlatReceiverBase
{
  protected:
    const Ieee80211ModeSet *modeSet = nullptr;
    const IIeee80211Band *band = nullptr;
    const Ieee80211Channel *channel = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;

    virtual Packet *computeReceivedPacket(const ISnir *snir, bool isReceptionSuccessful) const override;
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;
    void validateAmpduErrorModel(const Ieee80211ModeSet *modes) const;

    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;

  public:
    virtual ~Ieee80211Receiver();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual void setModeSet(const Ieee80211ModeSet *modeSet);
    virtual void setBand(const IIeee80211Band *band);
    virtual void setChannel(const Ieee80211Channel *channel);
    virtual void setChannelNumber(int channelNumber);
    virtual bool isHtChannelWidthSupported(Hz channelWidth) const;
    /**
     * The MAC calls this method before it updates local HT capabilities in the MIB.
     * For HT, a present error model must implement IIeee80211AmpduErrorModel and cover every advertised mode in the supplied bounds.
     * The bounds include the supplied channel widths, spatial stream limit, and every guard interval in modes.
     * Overrides must preserve this check and throw cRuntimeError before the update for an unsupported mode.
     * An absent error model needs no probability coverage.
     * Validation must not change capability state or consume random draws.
     */
    virtual void validateHtReceptionCapabilities(const Ieee80211ModeSet *modes, const std::set<Hz>& channelWidths, int spatialStreamLimit) const;
};

} // namespace physicallayer

} // namespace inet

#endif

