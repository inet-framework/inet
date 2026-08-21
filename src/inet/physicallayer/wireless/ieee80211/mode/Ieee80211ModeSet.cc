//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

#include <algorithm>

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ErpOfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211FhssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211IrMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h"

namespace inet {

namespace physicallayer {

Register_Abstract_Class(Ieee80211ModeSet);

std::vector<Ieee80211ModeSet::Entry> Ieee80211ModeSet::completeHtGuardIntervalVariants(const char *name, const std::vector<Entry>& entries)
{
    if (strcmp(name, "n(mixed-2.4Ghz)"))
        return entries;

    std::vector<Entry> completeEntries = entries;
    // IEEE Std 802.11-2024, Table 19-6 defines the 800 ns and 400 ns GIs.
    // Add only the alternate GI for each mode explicitly declared above;
    // declaration order, mandatory flags, and the historical catalog remain
    // authoritative for this operation mode.
    auto numberOfBaseEntries = completeEntries.size();
    for (size_t index = 0; index < numberOfBaseEntries; index++) {
        auto htMode = check_and_cast<const Ieee80211HtMode *>(completeEntries[index].mode);
        auto dataMode = htMode->getDataMode();
        auto alternateGuardInterval = dataMode->getGuardIntervalType() == Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG ?
                Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT : Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG;
        completeEntries.push_back({false, Ieee80211HtCompliantModes::getCompliantMode(
                dataMode->getModulationAndCodingScheme(), htMode->getCenterFrequencyMode(),
                htMode->getPreambleMode()->getPreambleFormat(), alternateGuardInterval)});
    }
    return completeEntries;
}

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
    // TODO slotTime, cwMin, cwMax must be identical in all modes
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
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode54Mbps }, // TODO ERP-CCK, ERP-PBCC, DSSS-OFDM
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
    Ieee80211ModeSet("n(mixed-2.4Ghz)", { // Completed below with all legal HT MCS and GI variants
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
    }),
    Ieee80211ModeSet("ac", {
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW20MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW20MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW40MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW80MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss2, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss3, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss4, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss5, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss6, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss7, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs1BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs2BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs3BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs4BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs5BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs6BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs7BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs8BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs9BW160MHzNss8, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT) },
}),}; });

Ieee80211ModeSet::Ieee80211ModeSet(const char *name, const std::vector<Entry> entries) :
    name(name),
    entries(completeHtGuardIntervalVariants(name, entries))
{
    std::vector<Entry> *nonConstEntries = const_cast<std::vector<Entry> *>(&this->entries);
    // Keep equal-bitrate modes in declaration order because unqualified lookups
    // intentionally preserve the historically preferred mode.
    // NOTE: This depends on (a) base entries being declared before appended variants
    // and (b) std::stable_sort preserving that order for ties. Future catalog edits
    // that reorder completeHtGuardIntervalVariants or the base list may break this invariant.
    std::stable_sort(nonConstEntries->begin(), nonConstEntries->end(), EntryNetBitrateComparator());
    auto referenceMode = entries[0].mode;
    for (auto entry : entries) {
        auto mode = entry.mode;
        if (mode->getSifsTime() != referenceMode->getSifsTime() ||
            mode->getSlotTime() != referenceMode->getSlotTime() ||
            mode->getPhyRxStartDelay() != referenceMode->getPhyRxStartDelay())
        {
            // FIXME throw cRuntimeError("Sifs, slot and phyRxStartDelay time must be identical within a ModeSet");
        }
    }
}

int Ieee80211ModeSet::findModeIndex(const IIeee80211Mode *mode) const
{
    for (size_t index = 0; index < entries.size(); index++)
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

const IIeee80211Mode *Ieee80211ModeSet::findCompatibleMode(const IIeee80211Mode *mode) const
{
    if (mode == nullptr)
        return nullptr;

    const auto sourceDataMode = mode->getDataMode();
    const auto sourceBitrate = sourceDataMode->getNetBitrate();
    const auto sourceBandwidth = sourceDataMode->getBandwidth();
    const auto sourceGuardInterval = sourceDataMode->getGuardInterval();
    const auto minBitrate = sourceBitrate - Mbps(0.05);
    const auto maxBitrate = sourceBitrate + Mbps(0.05);
    for (const auto& entry : entries) {
        const auto candidateDataMode = entry.mode->getDataMode();
        const auto candidateBandwidth = candidateDataMode->getBandwidth();
        const auto candidateGuardInterval = candidateDataMode->getGuardInterval();
        const bool bandwidthMatches = (std::isnan(sourceBandwidth.get()) && std::isnan(candidateBandwidth.get())) ||
                (!std::isnan(sourceBandwidth.get()) && !std::isnan(candidateBandwidth.get()) && sourceBandwidth == candidateBandwidth);
        // GI = -1 indicates unconstrained guard interval (e.g., non-OFDM modes).
        // Treat GI = -1 as matching any candidate GI, and require exact match when both are >= 0.
        const bool guardIntervalMatches = (sourceGuardInterval < SIMTIME_ZERO) || (candidateGuardInterval < SIMTIME_ZERO) ||
                (sourceGuardInterval >= SIMTIME_ZERO && candidateGuardInterval == sourceGuardInterval);
        if (minBitrate <= candidateDataMode->getNetBitrate() && candidateDataMode->getNetBitrate() <= maxBitrate &&
            bandwidthMatches && candidateDataMode->getNumberOfSpatialStreams() == sourceDataMode->getNumberOfSpatialStreams() &&
            guardIntervalMatches)
            return entry.mode;
    }
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::findMode(bps bitrate, Hz bandwidth, int numSpatialStreams, simtime_t guardInterval) const
{
    return findMode(bitrate - Mbps(0.05), bitrate + Mbps(0.05), bandwidth, numSpatialStreams, guardInterval);
}

const IIeee80211Mode *Ieee80211ModeSet::findMode(bps minBitrate, bps maxBitrate, Hz bandwidth, int numSpatialStreams, simtime_t guardInterval) const
{
    for (size_t index = 0; index < entries.size(); index++) {
        auto mode = entries[index].mode;
        auto dataMode = mode->getDataMode();
        auto bitrate = dataMode->getNetBitrate();
        bool guardIntervalMatches = guardInterval < SIMTIME_ZERO ||
                dataMode->getGuardInterval() == guardInterval;
        if (minBitrate <= bitrate && bitrate <= maxBitrate &&
            (std::isnan(bandwidth.get()) || dataMode->getBandwidth() == bandwidth) &&
            (numSpatialStreams == -1 || dataMode->getNumberOfSpatialStreams() == numSpatialStreams) &&
            guardIntervalMatches)
        {
            return entries[index].mode;
        }
    }
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getMode(bps bitrate, Hz bandwidth, int numSpatialStreams, simtime_t guardInterval) const
{
    const IIeee80211Mode *mode = getMode(bitrate - Mbps(0.05), bitrate + Mbps(0.05), bandwidth, numSpatialStreams, guardInterval);
    if (mode == nullptr)
        throw cRuntimeError("Unknown mode for bitrate %g bps, bandwidth %g Hz, %d spatial streams, and %s guard interval in operation mode '%s'",
                bitrate.get(), bandwidth.get(), numSpatialStreams, guardInterval.str().c_str(), getName());
    else
        return mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getMode(bps minBitrate, bps maxBitrate, Hz bandwidth, int numSpatialStreams, simtime_t guardInterval) const
{
    const IIeee80211Mode *mode = findMode(minBitrate, maxBitrate, bandwidth, numSpatialStreams, guardInterval);
    if (mode == nullptr)
        throw cRuntimeError("Unknown mode for bitrate range (%g - %g) bps, bandwidth %g Hz, %d spatial streams, and %s guard interval in operation mode '%s'",
                minBitrate.get(), maxBitrate.get(), bandwidth.get(), numSpatialStreams, guardInterval.str().c_str(), getName());
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
    if (index > 0) {
        auto bitrate = mode->getDataMode()->getNetBitrate();
        for (int i = index - 1; i >= 0; i--)
            if (entries[i].mode->getDataMode()->getNetBitrate() < bitrate)
                return entries[i].mode;
    }
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFasterMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index >= 0) {
        auto bitrate = mode->getDataMode()->getNetBitrate();
        for (size_t i = index + 1; i < entries.size(); i++)
            if (entries[i].mode->getDataMode()->getNetBitrate() > bitrate)
                return entries[i].mode;
    }
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowestMandatoryMode() const
{
    for (size_t i = 0; i < entries.size(); i++)
        if (entries[i].isMandatory)
            return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFastestMandatoryMode() const
{
    for (int i = (int)entries.size() - 1; i >= 0; i--)
        if (entries[i].isMandatory)
            return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getMandatoryModeAtOrBelow(const IIeee80211Mode *mode) const
{
    // Returns the highest-bitrate mandatory mode whose bitrate is <= the given mode's bitrate.
    // For equal-bitrate mandatory modes, returns the first-encountered entry (strict > comparison).
    // This may return a different mode object than the input when the input is mandatory and
    // shares bitrate with another mandatory mode, but the resulting rate is behavior-equivalent.
    const auto bitrate = mode->getDataMode()->getNetBitrate();
    const IIeee80211Mode *result = nullptr;
    for (const auto& entry : entries) {
        const auto entryBitrate = entry.mode->getDataMode()->getNetBitrate();
        if (entry.isMandatory && entryBitrate <= bitrate &&
            (result == nullptr || entryBitrate > result->getDataMode()->getNetBitrate()))
            result = entry.mode;
    }
    return result;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowerMandatoryMode(const IIeee80211Mode *mode) const
{
    const auto bitrate = mode->getDataMode()->getNetBitrate();
    const IIeee80211Mode *result = nullptr;
    for (const auto& entry : entries) {
        const auto entryBitrate = entry.mode->getDataMode()->getNetBitrate();
        if (entry.isMandatory && entryBitrate < bitrate &&
            (result == nullptr || entryBitrate > result->getDataMode()->getNetBitrate()))
            result = entry.mode;
    }
    return result;
}

const IIeee80211Mode *Ieee80211ModeSet::getFasterMandatoryMode(const IIeee80211Mode *mode) const
{
    const auto bitrate = mode->getDataMode()->getNetBitrate();
    const IIeee80211Mode *result = nullptr;
    for (const auto& entry : entries) {
        const auto entryBitrate = entry.mode->getDataMode()->getNetBitrate();
        if (entry.isMandatory && entryBitrate > bitrate &&
            (result == nullptr || entryBitrate < result->getDataMode()->getNetBitrate()))
            result = entry.mode;
    }
    return result;
}

const Ieee80211ModeSet *Ieee80211ModeSet::findModeSet(const char *mode)
{
    for (size_t index = 0; index < (&modeSets)->size(); index++) {
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
        for (size_t index = 0; index < (&modeSets)->size(); index++) {
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
