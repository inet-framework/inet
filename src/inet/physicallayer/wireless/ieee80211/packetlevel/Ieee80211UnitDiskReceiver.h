//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211UNITDISKRECEIVER_H
#define __INET_IEEE80211UNITDISKRECEIVER_H

#include "inet/physicallayer/wireless/unitdisk/UnitDiskReceiver.h"

namespace inet {

namespace physicallayer {

// TODO Ieee80211ReceiverBase
class INET_API Ieee80211UnitDiskReceiver : public UnitDiskReceiver
{
  protected:
    virtual void initialize(int stage) override;

    virtual bool computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const override;
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;

  public:
    Ieee80211UnitDiskReceiver();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReceptionResult *computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

