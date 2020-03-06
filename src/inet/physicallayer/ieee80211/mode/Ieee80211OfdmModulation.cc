//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"
#include "inet/physicallayer/modulation/BpskModulation.h"
#include "inet/physicallayer/modulation/Qam16Modulation.h"
#include "inet/physicallayer/modulation/Qam256Modulation.h"
#include "inet/physicallayer/modulation/Qam64Modulation.h"
#include "inet/physicallayer/modulation/QbpskModulation.h"
#include "inet/physicallayer/modulation/QpskModulation.h"

namespace inet {
namespace physicallayer {

Ieee80211OfdmModulation::Ieee80211OfdmModulation(int numSubcarriers, const ApskModulationBase* subcarrierModulation) :
    numSubcarriers(numSubcarriers),
    subcarrierModulation(subcarrierModulation)
{
}

std::ostream& Ieee80211OfdmModulation::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OfdmModulation";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", subcarrierModulation = " << printObjectToString(subcarrierModulation, level + 1);
    return stream;
}

// Modulations
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::subcarriers52QbpskModulation(52, &QbpskModulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::subcarriers52BpskModulation(52, &BpskModulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::subcarriers52QpskModulation(52, &QpskModulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::subcarriers52Qam16Modulation(52, &Qam16Modulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::subcarriers52Qam64Modulation(52, &Qam64Modulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::subcarriers52Qam256Modulation(52, &Qam256Modulation::singleton);

} /* namespace physicallayer */
} /* namespace inet */
