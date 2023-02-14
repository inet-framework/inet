//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKTRANSMITTERANALOGMODEL_H
#define __INET_UNITDISKTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceiverAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskListening.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ListeningDecision.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskReceiverAnalogModel : public ReceiverAnalogModelBase, public IReceiverAnalogModel
{
  protected:
    bool ignoreInterference = false;

  protected:
    virtual void initialize(int stage) override {
        if (stage == INITSTAGE_LOCAL)
            ignoreInterference = par("ignoreInterference");
    }

  public:
    virtual IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const override {
        return new UnitDiskListening(radio, startTime, endTime, startPosition, endPosition);
    }

    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const override {
        auto interferingReceptions = interference->getInterferingReceptions();
        for (auto interferingReception : *interferingReceptions) {
            auto interferingPower = check_and_cast<const UnitDiskReceptionAnalogModel *>(interferingReception->getAnalogModel())->getPower();
            if (interferingPower != UnitDiskReceptionAnalogModel::POWER_UNDETECTABLE)
                return new ListeningDecision(listening, true);
        }
        return new ListeningDecision(listening, false);
    }

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const override {
        auto power = check_and_cast<const UnitDiskReceptionAnalogModel *>(reception->getAnalogModel())->getPower();
        if (power == UnitDiskReceptionAnalogModel::POWER_RECEIVABLE) {
            if (ignoreInterference)
                return true;
            else {
                auto startTime = reception->getStartTime(part);
                auto endTime = reception->getEndTime(part);
                auto interferingReceptions = interference->getInterferingReceptions();
                for (auto interferingReception : *interferingReceptions) {
                    auto interferingPower = check_and_cast<const UnitDiskReceptionAnalogModel *>(interferingReception->getAnalogModel())->getPower();
                    if (interferingPower >= UnitDiskReceptionAnalogModel::POWER_INTERFERING && startTime <= interferingReception->getEndTime() && endTime >= interferingReception->getStartTime())
                        return false;
                }
                return true;
            }
        }
        else
            return false;
    }

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

