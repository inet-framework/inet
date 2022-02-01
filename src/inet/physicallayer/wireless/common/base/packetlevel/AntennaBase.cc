//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/AntennaBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace physicallayer {

AntennaBase::AntennaBase() :
    mobility(nullptr),
    numAntennas(-1)
{
}

void AntennaBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        mobility = check_and_cast_nullable<IMobility *>(getSubmodule("mobility"));
        if (mobility == nullptr)
            mobility = getModuleFromPar<IMobility>(par("mobilityModule"), this);
        numAntennas = par("numAntennas");
    }
}

std::ostream& AntennaBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream;
}

} // namespace physicallayer

} // namespace inet

