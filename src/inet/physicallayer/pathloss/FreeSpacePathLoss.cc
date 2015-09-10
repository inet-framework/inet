/***************************************************************************
* author:      Oliver Graute, Andreas Kuntz, Felix Schmidt-Eisenlohr
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
*              This program is free software; you can redistribute it
*              and/or modify it under the terms of the GNU General Public
*              License as published by the Free Software Foundation; either
*              version 2 of the License, or (at your option) any later
*              version.
*              For further information see file COPYING
*              in the top level directory
***************************************************************************/

#include "inet/physicallayer/pathloss/FreeSpacePathLoss.h"

namespace inet {

namespace physicallayer {

Define_Module(FreeSpacePathLoss);

FreeSpacePathLoss::FreeSpacePathLoss() :
    alpha(2.0),
    systemLoss(1.0)
{
}

void FreeSpacePathLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        alpha = par("alpha");
        systemLoss = math::dB2fraction(par("systemLoss"));
    }
}

std::ostream& FreeSpacePathLoss::printToStream(std::ostream& stream, int level) const
{
    stream << "FreeSpacePathLoss";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", alpha = " << alpha
               << ", systemLoss = " << systemLoss;
    return stream;
}

double FreeSpacePathLoss::computeFreeSpacePathLoss(m waveLength, m distance, double alpha, double systemLoss) const
{
    // factor = (waveLength / distance) ^ alpha / (16 * pi ^ 2 * systemLoss)
    double ratio = (waveLength / distance).get();
    // this check allows to get the same result from the GPU and the CPU when the alpha is exactly 2
    double raisedRatio = alpha == 2.0 ? ratio * ratio : pow(ratio, alpha);
    return distance.get() == 0.0 ? 1.0 : raisedRatio / (16.0 * M_PI * M_PI * systemLoss);
}

double FreeSpacePathLoss::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m waveLength = propagationSpeed / frequency;
    return computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
}

m FreeSpacePathLoss::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    m waveLength = propagationSpeed / frequency;
    return waveLength / pow(loss * 16.0 * M_PI * M_PI * systemLoss, 1.0 / alpha);
}

} // namespace physicallayer

} // namespace inet

