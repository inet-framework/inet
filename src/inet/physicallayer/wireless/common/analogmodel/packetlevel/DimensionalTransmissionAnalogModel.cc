//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmissionAnalogModel.h"

namespace inet {
namespace physicallayer {

DimensionalTransmissionAnalogModel::DimensionalTransmissionAnalogModel(Hz centerFrequency, Hz bandwidth, const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& powerFunction) :
    centerFrequency(centerFrequency), bandwidth(bandwidth), powerFunction(powerFunction)
{
    ASSERT(centerFrequency > Hz(0));
    ASSERT(bandwidth > Hz(0));
    ASSERT(powerFunction->getMax() > WpHz(0));
}

std::ostream& DimensionalTransmissionAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "DimensionalTransmission";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(powerFunction);
    return stream;
}

} // namespace physicallayer
} // namespace inet

