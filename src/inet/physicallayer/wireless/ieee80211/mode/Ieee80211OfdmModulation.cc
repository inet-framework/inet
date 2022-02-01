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

Ieee80211OfdmModulation::Ieee80211OfdmModulation(const ApskModulationBase *subcarrierModulation) :
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
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::qbpskModulation(&QbpskModulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::bpskModulation(&BpskModulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::qpskModulation(&QpskModulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::qam16Modulation(&Qam16Modulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::qam64Modulation(&Qam64Modulation::singleton);
const Ieee80211OfdmModulation Ieee80211OfdmCompliantModulations::qam256Modulation(&Qam256Modulation::singleton);

} /* namespace physicallayer */
} /* namespace inet */

