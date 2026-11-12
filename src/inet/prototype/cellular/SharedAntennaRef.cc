//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/SharedAntennaRef.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace cellular {

Define_Module(SharedAntennaRef);

const IAntenna *SharedAntennaRef::getAntenna() const
{
    if (antenna == nullptr) {
        auto self = const_cast<SharedAntennaRef *>(this);
        antenna = getModuleFromPar<IAntenna>(self->par("antennaModule"), self);
    }
    return antenna;
}

std::ostream& SharedAntennaRef::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SharedAntennaRef";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(antennaModule, par("antennaModule").stringValue());
    return stream;
}

} // namespace cellular

} // namespace inet

