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

#include "WeibullFading.h"

namespace inet {

namespace physicallayer {

Define_Module(WeibullFading);

WeibullFading::WeibullFading()
{
}

void WeibullFading::printToStream(std::ostream& stream) const
{
    stream << "Weibull fading";
}

void WeibullFading::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // TODO: variables
    }
}

double WeibullFading::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    throw cRuntimeError("Not implemented");
}

} // namespace physicallayer

} // namespace inet

