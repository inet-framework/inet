//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandDimensionalReceiver.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154NarrowbandDimensionalReceiver);

Ieee802154NarrowbandDimensionalReceiver::Ieee802154NarrowbandDimensionalReceiver() :
    FlatReceiverBase()
{
}

void Ieee802154NarrowbandDimensionalReceiver::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minInterferencePower = mW(math::dBmW2mW(par("minInterferencePower")));
    }
}

std::ostream& Ieee802154NarrowbandDimensionalReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee802154NarrowbandDimensionalReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

