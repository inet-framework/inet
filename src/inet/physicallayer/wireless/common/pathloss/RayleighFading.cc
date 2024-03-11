//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#include "inet/physicallayer/wireless/common/pathloss/RayleighFading.h"

namespace inet {

namespace physicallayer {

Define_Module(RayleighFading);

std::ostream& RayleighFading::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "RayleighFading";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << "systemLoss = " << systemLoss;
    return stream;
}

double RayleighFading::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m waveLength = propagationSpeed / frequency;
    double freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
    double x = normal(0, 1);
    double y = normal(0, 1);
    return freeSpacePathLoss * sqrt(x * x + y * y);
}

} // namespace physicallayer

} // namespace inet

