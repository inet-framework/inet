// Copyright (C) 2012 OpenSim Ltd
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

#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211FHSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211IRMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ERPOFDMMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTMode.h"
#include <algorithm>

namespace inet {

namespace physicallayer {

const DelayedInitializer<std::vector<Ieee80211ModeSet>> Ieee80211ModeSet::modeSets([]() { return new std::vector<Ieee80211ModeSet> {
    Ieee80211ModeSet("a", {
        { true, &Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS20MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS20MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS20MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS20MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS20MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode36Mbps },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode48Mbps },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode54Mbps },
    }),
    Ieee80211ModeSet("b", {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble },
    }),
    Ieee80211ModeSet("g", {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode6Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode9Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode12Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode18Mbps },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode24Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode36Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode48Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode54Mbps }, // TODO: ERP-CCK, ERP-PBCC, DSSS-OFDM
    }),
    Ieee80211ModeSet("p", {
        { true, &Ieee80211OFDMCompliantModes::ofdmMode3MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode4_5MbpsCS10MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS10MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode27Mbps },
        }),
    Ieee80211ModeSet("n", { // This table is not complete; it only contains 2.4GHz homogeneous spatial streams, all mandatory and optional modes
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs0BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs1BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs2BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs3BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs4BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs5BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs6BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs7BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs8BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs9BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs10BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs11BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs12BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs13BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs14BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs15BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs16BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs17BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs18BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs19BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs20BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs21BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs22BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs23BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs24BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs25BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs26BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs27BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs28BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs29BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs30BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs31BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs0BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs1BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs2BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs3BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs4BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs5BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs6BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs7BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs8BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs9BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs10BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs11BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs12BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs13BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs14BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs15BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs16BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs17BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs18BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs19BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs20BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs21BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs22BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs23BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs24BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs25BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs26BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs27BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs28BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs29BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs30BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs31BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) }
}),}; });

Ieee80211ModeSet::Ieee80211ModeSet(const char *name, const std::vector<Entry> entries) :
    name(name),
    entries(entries)
{
    std::vector<Entry> *nonConstEntries = const_cast<std::vector<Entry> *>(&this->entries);
    std::stable_sort(nonConstEntries->begin(), nonConstEntries->end(), EntryNetBitrateComparator());
}

int Ieee80211ModeSet::findModeIndex(const IIeee80211Mode *mode) const
{
    for (int index = 0; index < (int)entries.size(); index++)
        if (entries[index].mode == mode)
            return index;
    return -1;
}

int Ieee80211ModeSet::getModeIndex(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index < 0)
        throw cRuntimeError("Unknown mode");
    else
        return index;
}

bool Ieee80211ModeSet::getIsMandatory(const IIeee80211Mode *mode) const
{
    return entries[getModeIndex(mode)].isMandatory;
}

const IIeee80211Mode *Ieee80211ModeSet::findMode(bps bitrate) const
{
    for (int index = 0; index < (int)entries.size(); index++) {
        const IIeee80211Mode *mode = entries[index].mode;
        if (mode->getDataMode()->getNetBitrate() == bitrate)
            return entries[index].mode;
    }
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getMode(bps bitrate) const
{
    const IIeee80211Mode *mode = findMode(bitrate);
    if (mode == nullptr)
        throw cRuntimeError("Unknown bitrate: %g in operation mode: '%s'", bitrate.get(), getName());
    else
        return mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowestMode() const
{
    return entries.front().mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getFastestMode() const
{
    return entries.back().mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowerMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index > 0)
        return entries[index - 1].mode;
    else
        return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFasterMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index >= 0 && index < (int)entries.size()-1)
        return entries[index + 1].mode;
    else
        return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowestMandatoryMode() const
{
    for (int i = 0; i < (int)entries.size(); i++)
        if (entries[i].isMandatory)
            return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFastestMandatoryMode() const
{
    for (int i = (int)entries.size()-1; i >= 0; i--)
        if (entries[i].isMandatory)
            return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowerMandatoryMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index > 0)
        for (int i = index-1; i >= 0; i--)
            if (entries[i].isMandatory)
                return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFasterMandatoryMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index > 0)
        for (int i = index+1; i < (int)entries.size(); i++)
            if (entries[i].isMandatory)
                return entries[i].mode;
    return nullptr;
}

const Ieee80211ModeSet *Ieee80211ModeSet::findModeSet(const char *mode)
{
    for (int index = 0; index < (int)(&modeSets)->size(); index++) {
        const Ieee80211ModeSet *modeSet = &(&modeSets)->at(index);
        if (strcmp(modeSet->getName(), mode) == 0)
            return modeSet;
    }
    return nullptr;
}

const Ieee80211ModeSet *Ieee80211ModeSet::getModeSet(const char *mode)
{
    const Ieee80211ModeSet *modeSet = findModeSet(mode);
    if (modeSet == nullptr)
        throw cRuntimeError("Unknown 802.11 operational mode: '%s'", mode);
    else
        return modeSet;
}

} // namespace physicallayer

} // namespace inet

