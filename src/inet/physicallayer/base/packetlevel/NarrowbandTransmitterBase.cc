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

#include "inet/physicallayer/base/packetlevel/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"

namespace inet {

namespace physicallayer {

NarrowbandTransmitterBase::NarrowbandTransmitterBase() :
    modulation(nullptr),
    carrierFrequency(Hz(NaN)),
    bandwidth(Hz(NaN))
{
}

void NarrowbandTransmitterBase::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        modulation = APSKModulationBase::findModulation(par("modulation"));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}

std::ostream& NarrowbandTransmitterBase::printToStream(std::ostream& stream, int level) const
{
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", modulation = " << printObjectToString(modulation, level - 1) 
               << ", carrierFrequency = " << carrierFrequency
               << ", bandwidth = " << bandwidth;
    return stream;
}

} // namespace physicallayer

} // namespace inet

