//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/physicallayer/wireless/ieee802154/packetlevel/Ieee802154NarrowbandScalarReceiver.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee802154NarrowbandScalarReceiver);

Ieee802154NarrowbandScalarReceiver::Ieee802154NarrowbandScalarReceiver() :
    FlatReceiverBase()
{
}

void Ieee802154NarrowbandScalarReceiver::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        minInterferencePower = mW(math::dBmW2mW(par("minInterferencePower")));
    }
}

std::ostream& Ieee802154NarrowbandScalarReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee802154NarrowbandScalarReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

