//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/antenna/IsotropicAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(IsotropicAntenna);

IsotropicAntenna::IsotropicAntenna() :
    AntennaBase(), gain(makeShared<AntennaGain>())
{
}

std::ostream& IsotropicAntenna::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "IsotropicAntenna";
    return AntennaBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

