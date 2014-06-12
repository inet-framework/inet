/***************************************************************************
 * author:      Andreas Kuntz
 *
 * copyright:   (c) 2009 Institute of Telematics, University of Karlsruhe (TH)
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

#include "NakagamiFading.h"

using namespace radio;

Register_Class(NakagamiFading);

NakagamiFading::NakagamiFading() :
    shapeFactor(1)
{
}

void NakagamiFading::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        shapeFactor = par("shapeFactor");
    }
}

void NakagamiFading::printToStream(std::ostream &stream) const
{
    stream << "Nakagami fading, "
           << "alpha = " << alpha << ", "
           << "system loss = " << systemLoss << ", "
           << "shape factor = " << shapeFactor;
}

double NakagamiFading::computePathLoss(mps propagationSpeed, Hz carrierFrequency, m distance) const
{
    m waveLength = propagationSpeed / carrierFrequency;
    double freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
    return gamma_d(shapeFactor, freeSpacePathLoss / 1000.0 / shapeFactor) * 1000.0;
}
