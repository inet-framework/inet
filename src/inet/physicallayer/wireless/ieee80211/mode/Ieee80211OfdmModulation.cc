//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam256Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QbpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"

namespace inet {
namespace physicallayer {

Ieee80211OfdmModulation::Ieee80211OfdmModulation(int numSubcarriers, const ApskModulationBase *subcarrierModulation) :
    numSubcarriers(numSubcarriers),
    subcarrierModulation(subcarrierModulation)
{
}

std::ostream& Ieee80211OfdmModulation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211OfdmModulation";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(subcarrierModulation, printFieldToString(subcarrierModulation, level + 1, evFlags));
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

