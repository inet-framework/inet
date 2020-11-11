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

#include "inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h"

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

std::ostream& FreeSpacePathLoss::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "FreeSpacePathLoss";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << EV_FIELD(systemLoss);
    return stream;
}

double FreeSpacePathLoss::computeFreeSpacePathLoss(m waveLength, m distance, double alpha, double systemLoss) const
{
    // factor = waveLength ^ 2 / (16 * PI ^ 2 * systemLoss * distance ^ alpha)
    return distance.get() == 0.0 ? 1.0 : (waveLength * waveLength).get() / (16 * M_PI * M_PI * systemLoss * pow(distance.get(), alpha));
}

double FreeSpacePathLoss::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    m waveLength = propagationSpeed / frequency;
    return computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
}

m FreeSpacePathLoss::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    // distance = (waveLength ^ 2 / (16 * PI ^ 2 * systemLoss * loss)) ^ (1 / alpha)
    m waveLength = propagationSpeed / frequency;
    return m(pow((waveLength * waveLength).get() / (16.0 * M_PI * M_PI * systemLoss * loss), 1.0 / alpha));
}

} // namespace physicallayer

} // namespace inet

