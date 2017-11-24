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

#include "inet/physicallayer/ieee80211/mode/Ieee80211DsssMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ErpOfdmMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211FhssMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HrDsssMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211IrMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"
#include <algorithm>

namespace inet {

namespace physicallayer {

Register_Abstract_Class(Ieee80211ModeSet);

const DelayedInitializer<std::vector<Ieee80211ModeSet>> Ieee80211ModeSet::modeSets([]() { return new std::vector<Ieee80211ModeSet> {
    Ieee80211ModeSet("a", {
        { true, &Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode9MbpsCS20MHz },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS20MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode18MbpsCS20MHz },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode24MbpsCS20MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode36Mbps },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode48Mbps },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode54Mbps },
    }),
    Ieee80211ModeSet("b", {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble },
    }),
    // TODO: slotTime, cwMin, cwMax must be identical in all modes
    Ieee80211ModeSet("g(mixed)", {
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
    Ieee80211ModeSet("g(erp)", {
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode6Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode9Mbps },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode12Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode18Mbps },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode24Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode36Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode48Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode54Mbps },
    }),
    Ieee80211ModeSet("p", {
        { true, &Ieee80211OfdmCompliantModes::ofdmMode3MbpsCS10MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode4_5MbpsCS10MHz },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS10MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode9MbpsCS10MHz },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS10MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode18MbpsCS10MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode24MbpsCS10MHz },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode27Mbps },
        }),
    Ieee80211ModeSet("n(mixed-2.4Ghz)", { // This table is not complete; it only contains 2.4GHz homogeneous spatial streams, all mandatory and optional modes
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs1BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs2BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs3BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs4BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs5BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs6BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs7BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs8BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs9BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs10BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs11BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs12BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs13BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs14BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs15BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs16BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs17BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs18BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs19BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs20BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs21BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs22BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs23BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs24BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs25BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs26BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs27BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs28BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs29BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs30BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs31BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs1BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs2BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs3BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs4BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs5BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs6BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs7BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs8BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs9BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs10BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs11BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs12BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs13BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs14BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs15BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs16BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs17BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs18BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs19BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs20BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs21BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs22BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs23BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs24BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs25BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs26BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs27BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs28BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs29BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs30BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs31BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) }
}),}; });

Ieee80211ModeSet::Ieee80211ModeSet(const char *name, const std::vector<Entry> entries) :
    name(name),
    entries(entries)
{
    std::vector<Entry> *nonConstEntries = const_cast<std::vector<Entry> *>(&this->entries);
    std::stable_sort(nonConstEntries->begin(), nonConstEntries->end(), EntryNetBitrateComparator());
    auto referenceMode = entries[0].mode;
    for (auto entry : entries) {
        auto mode = entry.mode;
        if (mode->getSifsTime() != referenceMode->getSifsTime() ||
            mode->getSlotTime() != referenceMode->getSlotTime() ||
            mode->getPhyRxStartDelay() != referenceMode->getPhyRxStartDelay())
        {
            // FIXME: throw cRuntimeError("Sifs, slot and phyRxStartDelay time must be identical within a ModeSet");
        }
    }
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
    if (index >= 0)
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
    if (modeSet == nullptr) {
        std::string validModeSets;
        for (int index = 0; index < (int)(&modeSets)->size(); index++) {
            const Ieee80211ModeSet *modeSet = &(&modeSets)->at(index);
            validModeSets += std::string("'") + modeSet->getName() + "' ";
        }
        throw cRuntimeError("Unknown 802.11 operational mode: '%s', valid modes are: %s", mode, validModeSets.c_str());
    }
    else
        return modeSet;
}

} // namespace physicallayer

} // namespace inet

