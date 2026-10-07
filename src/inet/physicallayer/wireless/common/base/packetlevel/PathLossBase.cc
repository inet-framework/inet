//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/PathLossBase.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/signal/PowerFunctions.h"

namespace inet {

namespace physicallayer {

Ptr<const IFunction<double, Domain<Hz>>> PathLossBase::computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    mps propagationSpeed = transmission->getMedium()->getPropagation()->getPropagationSpeed();
    m distance = m(arrival->getStartPosition().distance(transmission->getStartPosition()));
    return makeShared<DistancePathLossFunction>(this, propagationSpeed, distance);
}

} // namespace physicallayer

} // namespace inet

