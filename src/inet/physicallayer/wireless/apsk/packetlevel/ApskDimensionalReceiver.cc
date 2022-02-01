//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskDimensionalReceiver.h"

#include "inet/physicallayer/wireless/apsk/packetlevel/ApskDimensionalTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalSnir.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskDimensionalReceiver);

ApskDimensionalReceiver::ApskDimensionalReceiver() :
    FlatReceiverBase()
{
}

std::ostream& ApskDimensionalReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskDimensionalReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

bool ApskDimensionalReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto apskTransmission = dynamic_cast<const ApskDimensionalTransmission *>(transmission);
    return apskTransmission && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool ApskDimensionalReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto apksTransmission = dynamic_cast<const ApskDimensionalTransmission *>(reception->getTransmission());
    return apksTransmission && FlatReceiverBase::computeIsReceptionPossible(listening, reception, part);
}

} // namespace physicallayer

} // namespace inet

