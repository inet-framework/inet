//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKRECEIVER_H
#define __INET_UNITDISKRECEIVER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the UnitDiskReceiver model, see the NED file for details.
 */
class INET_API UnitDiskReceiver : public ReceiverBase
{
  protected:
    bool ignoreInterference;

  protected:
    virtual void initialize(int stage) override;

  public:
    UnitDiskReceiver();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;

    virtual const IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override;
    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override;
    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

