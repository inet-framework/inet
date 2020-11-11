//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

