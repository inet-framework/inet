//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/antenna/DipoleAntenna.h"

namespace inet {

namespace physicallayer {

Define_Module(DipoleAntenna);

DipoleAntenna::DipoleAntenna() :
    AntennaBase(),
    length(NaN)
{
}

void DipoleAntenna::initialize(int stage)
{
    AntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        length = m(par("length"));
}

double DipoleAntenna::computeGain(EulerAngles direction) const
{
    double q = sin(direction.beta - M_PI_2);
    return 1.5 * q * q;
}

std::ostream& DipoleAntenna::printToStream(std::ostream& stream, int level) const
{
    stream << "DipoleAntenna";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", length = " << length;
    return AntennaBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

