//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKTRANSMITTERANALOGMODEL_H
#define __INET_UNITDISKTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceiverAnalogModel.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskListening.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskReceiverAnalogModel : public ReceiverAnalogModelBase, public IReceiverAnalogModel
{
  protected:
    // TODO: ignoreInterference?

  protected:
    virtual void initialize(int stage) override {
        if (stage == INITSTAGE_LOCAL) {
            // TODO: ignoreInterference?
        }
    }

  public:
    // TODO: ignoreInterference?

    virtual IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override {
        return new UnitDiskListening(radio, startTime, endTime, startPosition, endPosition);
    }

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

