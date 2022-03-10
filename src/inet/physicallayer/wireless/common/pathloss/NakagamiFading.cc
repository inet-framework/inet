//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2009 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#include "inet/physicallayer/wireless/common/pathloss/NakagamiFading.h"

namespace inet {

namespace physicallayer {

Define_Module(NakagamiFading);

NakagamiFading::NakagamiFading() :
    shapeFactor(1)
{
}

void NakagamiFading::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        shapeFactor = par("shapeFactor");
    }
}

std::ostream& NakagamiFading::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "NakagamiFading";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << EV_FIELD(systemLoss)
               << EV_FIELD(shapeFactor);
    return stream;
}

double NakagamiFading::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m waveLength = propagationSpeed / frequency;
    double freeSpacePathLoss = computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
    return gamma_d(shapeFactor, freeSpacePathLoss / 1000.0 / shapeFactor) * 1000.0;
}

} // namespace physicallayer

} // namespace inet

