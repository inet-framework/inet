//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskScalarReceiver.h"

#include "inet/physicallayer/wireless/apsk/packetlevel/ApskTransmission.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskScalarReceiver);

ApskScalarReceiver::ApskScalarReceiver() :
    FlatReceiverBase()
{
}

std::ostream& ApskScalarReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskScalarReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

bool ApskScalarReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto apskTransmission = dynamic_cast<const ApskTransmission *>(transmission);
    return apskTransmission && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool ApskScalarReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto apksTransmission = dynamic_cast<const ApskTransmission *>(reception->getTransmission());
    return apksTransmission && getAnalogModel()->computeIsReceptionPossible(listening, reception, part);
}

} // namespace physicallayer

} // namespace inet

