//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#include "inet/physicallayer/wireless/common/pathloss/LogNormalShadowing.h"

namespace inet {

namespace physicallayer {

Define_Module(LogNormalShadowing);

LogNormalShadowing::LogNormalShadowing() :
    sigma(1)
{
}

void LogNormalShadowing::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        sigma = par("sigma");
    }
}

std::ostream& LogNormalShadowing::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LogNormalShadowing";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << EV_FIELD(systemLoss)
               << EV_FIELD(sigma);
    return stream;
}

double LogNormalShadowing::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m d0 = m(1.0);
    // reference path loss
    double freeSpacePathLoss = computeFreeSpacePathLoss(propagationSpeed / frequency, d0, alpha, systemLoss);
    double PL_d0_db = 10.0 * log10(1 / freeSpacePathLoss);
    // path loss at distance d + normal distribution with sigma standard deviation
    double PL_db = PL_d0_db + 10 * alpha * log10(unit(distance / d0).get()) + normal(0.0, sigma);
    return math::dB2fraction(-PL_db);
}

} // namespace physicallayer

} // namespace inet

