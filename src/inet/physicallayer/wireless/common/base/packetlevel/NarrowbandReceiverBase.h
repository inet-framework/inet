//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NARROWBANDRECEIVERBASE_H
#define __INET_NARROWBANDRECEIVERBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/SnirReceiverBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IErrorModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandReceiverBase : public SnirReceiverBase
{
  protected:
    const IModulation *modulation;
    Hz centerFrequency;
    Hz bandwidth;

  protected:
    virtual void initialize(int stage) override;

  public:
    NarrowbandReceiverBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;

    virtual const IReceptionDecision *computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;

    virtual const IModulation *getModulation() const { return modulation; }
    virtual void setModulation(const IModulation *modulation) { this->modulation = modulation; }

    virtual Hz getCenterFrequency() const { return centerFrequency; }
    virtual void setCenterFrequency(Hz centerFrequency) { this->centerFrequency = centerFrequency; }

    virtual Hz getBandwidth() const { return bandwidth; }
    virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif

