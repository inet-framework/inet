//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandReceiver.h"
#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154Transmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154NarrowbandReceiver);

Ieee802154NarrowbandReceiver::Ieee802154NarrowbandReceiver() :
    FlatReceiverBase()
{
}

void Ieee802154NarrowbandReceiver::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minInterferencePower = mW(math::dBmW2mW(par("minInterferencePower")));
    }
}

bool Ieee802154NarrowbandReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto ieee802154Transmission = dynamic_cast<const Ieee802154Transmission *>(transmission);
    return ieee802154Transmission && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool Ieee802154NarrowbandReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto ieee802154Transmission = dynamic_cast<const Ieee802154Transmission *>(reception->getTransmission());
    return ieee802154Transmission && getAnalogModel()->computeIsReceptionPossible(listening, reception, part);
}

std::ostream& Ieee802154NarrowbandReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee802154NarrowbandReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

