//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskListening.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskReceiverAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskReceptionAnalogModel.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskReceiverAnalogModel);

void UnitDiskReceiverAnalogModel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        ignoreInterference = par("ignoreInterference");
}

IListening* UnitDiskReceiverAnalogModel::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord &startPosition, const Coord &endPosition) const
{
    return new UnitDiskListening(radio, startTime, endTime, startPosition, endPosition);
}

bool UnitDiskReceiverAnalogModel::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto power = check_and_cast<const UnitDiskReceptionAnalogModel *>(reception->getAnalogModel())->getPower();
    return power == UnitDiskReceptionAnalogModel::POWER_RECEIVABLE;
}

bool UnitDiskReceiverAnalogModel::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    auto power = check_and_cast<const UnitDiskReceptionAnalogModel*>(reception->getAnalogModel())->getPower();
    if (power == UnitDiskReceptionAnalogModel::POWER_RECEIVABLE) {
        if (ignoreInterference)
            return true;
        else {
            auto startTime = reception->getStartTime(part);
            auto endTime = reception->getEndTime(part);
            auto interferingReceptions = interference->getInterferingReceptions();
            for (auto interferingReception : *interferingReceptions) {
                auto interferingPower = check_and_cast<const UnitDiskReceptionAnalogModel*>(interferingReception->getAnalogModel())->getPower();
                if (interferingPower >= UnitDiskReceptionAnalogModel::POWER_INTERFERING && startTime <= interferingReception->getEndTime() && endTime >= interferingReception->getStartTime())
                    return false;
            }
            return true;
        }
    }
    else
        return false;
}

} // namespace physicallayer
} // namespace inet

