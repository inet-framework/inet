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

#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"

namespace inet {
namespace physicallayer {

Ieee80211OFDMModulation::Ieee80211OFDMModulation(const APSKModulationBase* modulation) :
        subcarrierModulation(modulation)
{
}

// Modulations
const Ieee80211OFDMModulation Ieee80211OFDMCompliantModulations::bpskModulation(&BPSKModulation::singleton);
const Ieee80211OFDMModulation Ieee80211OFDMCompliantModulations::qpskModulation(&QPSKModulation::singleton);
const Ieee80211OFDMModulation Ieee80211OFDMCompliantModulations::qam16Modulation(&QAM16Modulation::singleton);
const Ieee80211OFDMModulation Ieee80211OFDMCompliantModulations::qam64Modulation(&QAM64Modulation::singleton);

} /* namespace physicallayer */
} /* namespace inet */
