//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/PathLossBase.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

std::ostream& PathLossBase::DistanceReceptionPathLoss::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DistanceReceptionPathLoss";
    if (level <= PRINT_LEVEL_DEBUG)
        stream << EV_FIELD(distance);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(propagationSpeed) << EV_FIELD(pathLoss, printFieldToString(pathLoss, level + 1, evFlags));
    return stream;
}

Ptr<const IReceptionPathLoss> PathLossBase::computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    mps propagationSpeed = transmission->getMedium()->getPropagation()->getPropagationSpeed();
    m distance = m(arrival->getStartPosition().distance(transmission->getStartPosition()));
    return makeShared<DistanceReceptionPathLoss>(this, propagationSpeed, distance);
}

} // namespace physicallayer

} // namespace inet

