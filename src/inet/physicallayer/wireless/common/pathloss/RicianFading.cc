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

#include "inet/physicallayer/wireless/common/pathloss/RicianFading.h"

namespace inet {

namespace physicallayer {

Define_Module(RicianFading);

RicianFading::RicianFading() :
    k(1)
{
}

void RicianFading::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        k = math::dB2fraction(par("k"));
    }
}

std::ostream& RicianFading::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "RicianFading";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << ", system loss = " << systemLoss
               << EV_FIELD(k);
    return stream;
}

double RicianFading::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m waveLength = propagationSpeed / frequency;
    double c = 1.0 / (2.0 * (k + 1));
    double x = normal(0, 1);
    double y = normal(0, 1);
    double rr = c * ((x + sqrt(2 * k)) * (x + sqrt(2 * k)) + y * y);
    double freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
    return rr * freeSpacePathLoss;
}

} // namespace physicallayer

} // namespace inet

