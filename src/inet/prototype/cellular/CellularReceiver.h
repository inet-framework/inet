//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CELLULARRECEIVER_H
#define __INET_CELLULARRECEIVER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"

namespace inet {

namespace cellular {

using namespace inet::physicallayer;

/**
 * Receives the allocations addressed to its radio on one component carrier, and decides each
 * transport block with a CellularErrorModel.
 */
class INET_API CellularReceiver : public FlatReceiverBase
{
  protected:
    // workaround for C16 (concurrent receptions): the base class attempts a reception only if
    // no other one is in progress, so of the uplink allocations that reach a base station at
    // the same time, it would attempt only the first
    virtual bool computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const override;
    virtual bool computeIsReceptionSuccessful(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    // with the listening filter of the radio medium, this delivers each allocation only to its
    // intended receiver
    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;

    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
};

} // namespace cellular

} // namespace inet

#endif

