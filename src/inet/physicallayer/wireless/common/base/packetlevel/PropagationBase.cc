//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/PropagationBase.h"

namespace inet {

namespace physicallayer {

PropagationBase::PropagationBase() :
    propagationSpeed(mps(NaN)),
    arrivalComputationCount(0)
{
}

void PropagationBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        propagationSpeed = mps(par("propagationSpeed"));
}

std::ostream& PropagationBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(propagationSpeed);
    return stream;
}

void PropagationBase::finish()
{
    EV_INFO << "Radio signal arrival computation count = " << arrivalComputationCount << endl;
    recordScalar("Arrival computation count", arrivalComputationCount);
}

} // namespace physicallayer

} // namespace inet

