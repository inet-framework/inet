//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GENERICRECEIVER_H
#define __INET_GENERICRECEIVER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the GenericReceiver model, see the NED file for details.
 */
class INET_API GenericReceiver : public ReceiverBase
{
  protected:
    bool ignoreInterference = false;
    W energyDetection;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;

    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

