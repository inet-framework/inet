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

IListening* UnitDiskReceiverAnalogModel::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord &startPosition, const Coord &endPosition, Hz centerFrequency, Hz bandwidth) const
{
    // centerFrequency and bandwidth are ignored
    return new UnitDiskListening(radio, startTime, endTime, startPosition, endPosition);
}

bool UnitDiskReceiverAnalogModel::computeIsReceptionPossible(const IListening *listening, const IReception *reception, W sensitivity) const
{
    // sensitivity is ignored
    auto power = check_and_cast<const UnitDiskReceptionAnalogModel *>(reception->getAnalogModel())->getPower();
    return power == UnitDiskReceptionAnalogModel::POWER_RECEIVABLE;
}

} // namespace physicallayer
} // namespace inet

