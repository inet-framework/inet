//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskReceiverAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskReceptionAnalogModel.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskReceiverAnalogModel);

bool UnitDiskReceiverAnalogModel::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto power = check_and_cast<const UnitDiskReceptionAnalogModel *>(reception->getNewAnalogModel())->getPower();
    return power == UnitDiskReceptionAnalogModel::POWER_RECEIVABLE;
}

} // namespace physicallayer
} // namespace inet

