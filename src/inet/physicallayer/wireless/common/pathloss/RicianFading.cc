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

