//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <typeinfo>

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
    if (strcmp(name, "n(mixed-2.4Ghz)") && strcmp(name, "n(greenfield-2.4Ghz)"))
        return entries;

    std::vector<Entry> completeEntries = entries;
    // IEEE Std 802.11-2024, Table 19-6 defines the 800 ns and 400 ns GIs.
    // Add only the alternate GI for each mode explicitly declared above;
    // declaration order, mandatory flags, and the historical catalog remain
    // authoritative for this operation mode.
    auto numberOfBaseEntries = completeEntries.size();
    for (size_t index = 0; index < numberOfBaseEntries; index++) {
        auto htMode = dynamic_cast<const Ieee80211HtMode *>(completeEntries[index].mode);
        if (htMode == nullptr)
            continue;
        auto dataMode = htMode->getDataMode();
        auto alternateGuardInterval = dataMode->getGuardIntervalType() == Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG ?
                Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT : Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG;
        auto alternateMode = Ieee80211HtCompliantModes::getCompliantMode(
                dataMode->getModulationAndCodingScheme(), htMode->getCenterFrequencyMode(),
                htMode->getPreambleMode()->getPreambleFormat(), alternateGuardInterval);
        auto alternateAlreadyPresent = std::any_of(completeEntries.begin(), completeEntries.end(), [alternateMode](const Entry& entry) { return entry.mode == alternateMode; });
        if (!alternateAlreadyPresent)
            completeEntries.push_back({false, alternateMode});
    }
    return completeEntries;
}

#define HT_MODE_ENTRY(WIDTH, MCS, MANDATORY, FORMAT, GUARD_INTERVAL) \
    { MANDATORY, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs##MCS##BW##WIDTH##MHz, Ieee80211HtMode::BAND_2_4GHZ, FORMAT, GUARD_INTERVAL) },
#define HT_MODE_ENTRIES_20(FORMAT) \
    HT_MODE_ENTRY(20, 0, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 1, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 2, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 3, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 4, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 5, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 6, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 7, true, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) \
    HT_MODE_ENTRY(20, 8, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 9, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 10, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 11, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 12, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 13, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 14, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 15, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 16, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 17, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 18, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 19, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 20, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 21, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 22, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 23, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 24, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 25, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 26, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 27, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 28, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 29, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 30, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(20, 31, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT)
#define HT_MODE_ENTRIES_40(FORMAT) \
    HT_MODE_ENTRY(40, 0, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 1, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 2, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 3, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 4, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 5, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 6, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 7, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 8, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 9, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 10, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 11, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 12, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 13, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 14, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 15, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 16, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 17, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 18, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 19, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 20, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 21, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 22, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 23, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 24, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 25, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 26, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 27, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 28, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 29, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 30, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) \
    HT_MODE_ENTRY(40, 31, false, FORMAT, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT)

static std::vector<Ieee80211ModeSet::Entry> createHtEntries(Ieee80211HtPreambleMode::HighTroughputPreambleFormat preambleFormat)
{
    return {
        HT_MODE_ENTRIES_20(preambleFormat)
        HT_MODE_ENTRIES_40(preambleFormat)
    };
}

static std::vector<Ieee80211ModeSet::Entry> createHtSupportedEntries(Ieee80211HtPreambleMode::HighTroughputPreambleFormat preambleFormat)
{
    auto result = createHtEntries(preambleFormat);
    if (preambleFormat == Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD) {
        auto mixedEntries = createHtEntries(Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED);
        result.insert(result.end(), mixedEntries.begin(), mixedEntries.end());
    }
    // Every 2.4 GHz HT STA supports the mandatory Clause 16/18 modes, and a
    // Greenfield STA additionally supports HT-mixed PPDUs (IEEE 802.11-2024
    // 19.1.1 and 19.1.4). These are supplementary capabilities rather than
    // selectable operating modes, so they are kept out of createHtEntries().
    result.insert(result.end(), {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps, true },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode2MbpsShortPreamble },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckShortPreamble },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode6Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode9Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckShortPreamble },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode12Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode18Mbps },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode24Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode36Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode48Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode54Mbps },
    });
    return result;
}

#undef HT_MODE_ENTRIES_40
#undef HT_MODE_ENTRIES_20
#undef HT_MODE_ENTRY

OPP_THREAD_LOCAL const DelayedInitializer<std::vector<Ieee80211ModeSet>> Ieee80211ModeSet::modeSets([]() { return new std::vector<Ieee80211ModeSet> {
    Ieee80211ModeSet("a", {
        { true, &Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode9MbpsCS20MHz, true },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS20MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode18MbpsCS20MHz, true },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode24MbpsCS20MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode36Mbps, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode48Mbps, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode54Mbps, true },
    }, &Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz, PhyType::OFDM),
    Ieee80211ModeSet("b", {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps, true },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble, true },
    }, &Ieee80211DsssCompliantModes::dsssMode1Mbps, PhyType::HR_DSSS),
    // Mixed-ERP timing is intentionally anchored to the explicitly selected
    // DSSS reference mode; the entries do not need identical timing values.
    Ieee80211ModeSet("g(mixed)", {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps, true },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode6Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode9Mbps, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode12Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode18Mbps, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode24Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode36Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode48Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode54Mbps, true }, // TODO ERP-CCK, ERP-PBCC, DSSS-OFDM
    }, &Ieee80211DsssCompliantModes::dsssMode1Mbps, PhyType::ERP),
    Ieee80211ModeSet("g(erp)", {
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode6Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode9Mbps, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode12Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode18Mbps, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode24Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode36Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode48Mbps, true },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode54Mbps, true },
    }, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode6Mbps, PhyType::ERP),
    Ieee80211ModeSet("p", {
        { true, &Ieee80211OfdmCompliantModes::ofdmMode3MbpsCS10MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode4_5MbpsCS10MHz, true },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS10MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode9MbpsCS10MHz, true },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS10MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode18MbpsCS10MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode24MbpsCS10MHz, true },
        { false, &Ieee80211OfdmCompliantModes::ofdmMode27Mbps, true },
        }, &Ieee80211OfdmCompliantModes::ofdmMode3MbpsCS10MHz, PhyType::OFDM),
    // IEEE Std 802.11-2024, Table 9-224, 10.17, and 19.1.1: receiver MCS,
    // channel-width, and short-GI capabilities are advertised independently.
    // Keep every MCS 0..31 x 20/40 MHz x long/short GI combination selectable.
    Ieee80211ModeSet("n(mixed-2.4Ghz)", {
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs1BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs2BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs3BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs4BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs5BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs6BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs7BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs1BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs2BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs3BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs4BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs5BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs6BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs7BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
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
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs8BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs9BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs10BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs11BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs12BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs13BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs14BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs15BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs16BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs17BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs18BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs19BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs20BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs21BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs22BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs23BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs24BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs25BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs26BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs27BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs28BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs29BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs30BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs31BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
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
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs31BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs1BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs2BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs3BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs4BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs5BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs6BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs7BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs8BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs9BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs10BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs11BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs12BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs13BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs14BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs15BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs16BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs17BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs18BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs19BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs20BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs21BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs22BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs23BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs24BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs25BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs26BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs27BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs28BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs29BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs30BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs31BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        // IEEE Std 802.11-2024, 11.1.4.6 and 19.1.1: these legacy modes are
        // authoritative mode-set entries and are eligible for Supported Rates;
        // HT MCS entries above remain in the HT capabilities set.
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps, true },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode6Mbps, true },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode12Mbps, true },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode24Mbps, true }
    }, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG), PhyType::HT, true, createHtSupportedEntries(Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED)),
    Ieee80211ModeSet("n(greenfield-2.4Ghz)", {
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs1BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs2BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs3BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs4BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs5BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs6BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs7BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs8BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs9BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs10BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs11BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs12BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs13BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs14BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs15BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs16BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs17BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs18BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs19BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs20BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs21BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs22BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs23BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs24BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs25BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs26BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs27BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs28BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs29BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs30BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs31BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs1BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs2BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs3BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs4BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs5BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs6BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs7BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs8BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs9BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs10BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs11BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs12BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs13BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs14BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs15BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs16BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs17BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs18BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs19BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs20BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs21BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs22BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs23BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs24BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs25BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs26BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs27BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs28BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs29BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs30BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs31BW40MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT) }
    }, Ieee80211HtCompliantModes::getCompliantMode(&Ieee80211HtmcsTable::htMcs0BW20MHz, Ieee80211HtMode::BAND_2_4GHZ, Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD, Ieee80211HtModeBase::HT_GUARD_INTERVAL_LONG), PhyType::HT, true, createHtSupportedEntries(Ieee80211HtPreambleMode::HT_PREAMBLE_GREENFIELD)),
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
        // IEEE Std 802.11-2024, 9.4.2.3 and 11.1.4.6: these legacy OFDM modes
        // are eligible for Supported Rates; VHT MCS entries above are not.
        { true, &Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz, true },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode12MbpsCS20MHz, true },
        { true, &Ieee80211OfdmCompliantModes::ofdmMode24MbpsCS20MHz, true },
    // Intentional model limitation: unlike IEEE Std 802.11-2024, 11.38.1,
    // this VHT-only profile has no selectable HT modes.
    }, Ieee80211VhtCompliantModes::getCompliantMode(&Ieee80211VhtmcsTable::vhtMcs0BW20MHzNss1, Ieee80211VhtMode::BAND_5GHZ, Ieee80211VhtPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VhtModeBase::HT_GUARD_INTERVAL_LONG), PhyType::VHT),}; });

Ieee80211ModeSet::Ieee80211ModeSet(const char *name, const std::vector<Entry> entries, const IIeee80211Mode *referenceMode,
        PhyType phyType, bool htOperationSupported, const std::vector<Entry> supportedEntries) :
    name(name),
    entries(completeHtGuardIntervalVariants(name, entries)),
    phyType(phyType),
    referenceMode(referenceMode),
    htOperationSupported(htOperationSupported),
    supportedEntries(supportedEntries.empty() ? this->entries : completeHtGuardIntervalVariants(name, supportedEntries)),
    controlResponseModes(createControlResponseModes(this->supportedEntries)),
    htMixedControlResponseModes(createHtMixedControlResponseModes(this->supportedEntries)),
    nonHtControlResponseEntries(createNonHtControlResponseEntries(this->supportedEntries))
{
    if (this->entries.empty())
        throw cRuntimeError("IEEE 802.11 mode set '%s' must contain at least one mode", this->name.c_str());
    if (this->referenceMode == nullptr)
        throw cRuntimeError("IEEE 802.11 mode set '%s' has a null reference mode", this->name.c_str());
    int referenceModeCount = 0;
    for (const auto& entry : this->entries) {
        if (entry.mode == nullptr)
            throw cRuntimeError("IEEE 802.11 mode set '%s' contains a null mode", this->name.c_str());
        if (entry.mode == this->referenceMode)
            referenceModeCount++;
        if (entry.isLegacyOperational) {
            auto bitrate = entry.mode->getDataMode()->getNetBitrate().get<Mbps>();
            if (!std::isfinite(bitrate) || bitrate <= 0 || bitrate > 63.5)
                throw cRuntimeError("Mode '%s' is not representable as a legacy Supported Rate in IEEE 802.11 mode set '%s'", entry.mode->getName(), this->name.c_str());
        }
    }
    if (referenceModeCount == 0)
        throw cRuntimeError("Reference mode '%s' is not contained in IEEE 802.11 mode set '%s'", this->referenceMode->getName(), this->name.c_str());
    if (referenceModeCount > 1)
        throw cRuntimeError("Reference mode '%s' occurs %d times in IEEE 802.11 mode set '%s'", this->referenceMode->getName(), referenceModeCount, this->name.c_str());
    if (this->referenceMode->getSifsTime() <= SIMTIME_ZERO)
        throw cRuntimeError("Reference mode '%s' in IEEE 802.11 mode set '%s' has a non-positive SIFS time", this->referenceMode->getName(), this->name.c_str());
    if (this->referenceMode->getSlotTime() <= SIMTIME_ZERO)
        throw cRuntimeError("Reference mode '%s' in IEEE 802.11 mode set '%s' has a non-positive slot time", this->referenceMode->getName(), this->name.c_str());
    if (this->referenceMode->getPhyRxStartDelay() <= SIMTIME_ZERO)
        throw cRuntimeError("Reference mode '%s' in IEEE 802.11 mode set '%s' has a non-positive PHY RX start delay", this->referenceMode->getName(), this->name.c_str());
    if (this->referenceMode->getLegacyCwMin() < 0 || this->referenceMode->getLegacyCwMax() < this->referenceMode->getLegacyCwMin())
        throw cRuntimeError("Reference mode '%s' in IEEE 802.11 mode set '%s' has invalid contention window bounds", this->referenceMode->getName(), this->name.c_str());
    std::vector<Entry> *nonConstEntries = const_cast<std::vector<Entry> *>(&this->entries);
    // Keep equal-bitrate modes in declaration order because unqualified lookups
    // intentionally preserve the historically preferred mode.
    std::stable_sort(nonConstEntries->begin(), nonConstEntries->end(), EntryNetBitrateComparator());
    // Explicit Supported-Rates eligibility on the authoritative Entry keeps
    // HT/VHT MCSs out without concrete-type or name-based inference. The
    // modeled OperationalRateSet is mandatory-first (IEEE Std 802.11-2024,
    // 9.4.2.3 and 11.1.4.6); management splits it across the primary and
    // Extended Supported Rates elements when necessary.
    for (bool mandatory : {true, false}) {
        for (const auto& entry : this->supportedEntries) {
            if (entry.isLegacyOperational && entry.isMandatory == mandatory)
                this->legacyOperationalModes.push_back(entry.mode);
        }
    }
    // IEEE Std 802.11-2024, 9.4.2.3 and 11.1.4.6: an ordinary STA
    // advertises its applicable operational rates in Supported Rates. This
    // model has no selector-only S1G/CMMG mode-set exception.
    if (this->legacyOperationalModes.empty())
        throw cRuntimeError("IEEE 802.11 mode set '%s' must contain at least one legacy operational mode for Supported Rates", this->name.c_str());
    for (const auto *mode : this->legacyOperationalModes) {
        if (mode == nullptr)
            throw cRuntimeError("IEEE 802.11 mode set '%s' contains a null legacy operational mode", this->name.c_str());
        bool found = false;
        for (const auto& entry : this->supportedEntries) {
            if (entry.mode == mode && entry.isLegacyOperational) {
                found = true;
                break;
            }
        }
        if (!found)
            throw cRuntimeError("Legacy operational mode '%s' is not marked eligible in IEEE 802.11 mode set '%s'", mode->getName(), this->name.c_str());
    }
    for (const auto& entry : this->entries) {
        int mcsIndex = entry.mode->getHtMcsIndex();
        if (mcsIndex < 0)
            continue;
        if (mcsIndex >= 77)
            throw cRuntimeError("HT MCS index %d in mode '%s' is outside the modeled range 0..76 in mode set '%s'",
                    mcsIndex, entry.mode->getName(), this->name.c_str());
        htMcsSupported[mcsIndex] = true;
        htMcsMandatory[mcsIndex] = htMcsMandatory[mcsIndex] || entry.isMandatory;
        auto bandwidth = entry.mode->getDataMode()->getBandwidth();
        htSupportedChannelWidths.insert(bandwidth);
        if (entry.mode->isHtShortGuardInterval())
            htShortGuardIntervalChannelWidths.insert(bandwidth);
        if (entry.mode->isHtGreenfield())
            htGreenfieldSupported = true;
    }
    if (htOperationSupported) {
        for (int mcsIndex = 0; mcsIndex < 8; mcsIndex++)
            if (!htMcsSupported[mcsIndex] || !htMcsMandatory[mcsIndex])
                throw cRuntimeError("HT operation mode set '%s' must mark mandatory HT MCS %d as supported", this->name.c_str(), mcsIndex);
    }
}

Hz Ieee80211ModeSet::getMaximumChannelWidth() const
{
    Hz maximum(0);
    for (const auto& entry : entries)
        maximum = std::max(maximum, entry.mode->getDataMode()->getBandwidth());
    return maximum;
}

int Ieee80211ModeSet::getMaximumNumberOfSpatialStreams() const
{
    int maximum = 0;
    for (const auto& entry : entries)
        maximum = std::max(maximum, entry.mode->getDataMode()->getNumberOfSpatialStreams());
    return maximum;
}

int Ieee80211ModeSet::findModeIndex(const IIeee80211Mode *mode) const
{
    for (size_t index = 0; index < entries.size(); index++)
        if (entries[index].mode == mode)
            return index;
    return -1;
}

std::map<const IIeee80211Mode *, const IIeee80211Mode *> Ieee80211ModeSet::createControlResponseModes(const std::vector<Entry>& supportedEntries)
{
    std::map<const IIeee80211Mode *, const IIeee80211Mode *> result;
    // IEEE 802.11-2024 Table 9-230 defines the Basic HT-MCS Set as a BSS-
    // configured bitmap of MCS indexes. It is not modelled here, so use the
    // mandatory entries (currently the 20 MHz entries) as a bounded fallback
    // for the candidate indexes. Clause 10.6.6.5.3 selects CH_BANDWIDTH
    // separately, and the candidate filter below then keeps only modes at the
    // source bandwidth. Consequently, optional 40 MHz MCS 0..7 are treated as
    // candidate MCSs. A modelled Basic HT-MCS Set would replace this mandatory-
    // index fallback with the BSS-configured indexes; bandwidth filtering would
    // remain a separate step.
    std::set<unsigned int> mandatoryHtMcsIndexes;
    for (const auto& entry : supportedEntries) {
        auto mode = dynamic_cast<const Ieee80211HtMode *>(entry.mode);
        if (entry.isMandatory && mode != nullptr)
            mandatoryHtMcsIndexes.insert(mode->getDataMode()->getMcsIndex());
    }
    for (const auto& sourceEntry : supportedEntries) {
        auto source = dynamic_cast<const Ieee80211HtMode *>(sourceEntry.mode);
        if (source == nullptr)
            continue;
        std::vector<const Ieee80211HtMode *> candidates;
        for (const auto& candidateEntry : supportedEntries) {
            auto candidate = dynamic_cast<const Ieee80211HtMode *>(candidateEntry.mode);
            if (candidate != nullptr && candidate->getPreambleMode()->getPreambleFormat() == Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED &&
                candidate->getCenterFrequencyMode() == source->getCenterFrequencyMode() &&
                candidate->getDataMode()->getBandwidth() == source->getDataMode()->getBandwidth() &&
                mandatoryHtMcsIndexes.find(candidate->getDataMode()->getMcsIndex()) != mandatoryHtMcsIndexes.end() &&
                candidate->getDataMode()->getMcsIndex() <= source->getDataMode()->getMcsIndex() &&
                candidate->getDataMode()->getNumberOfSpatialStreams() <= source->getDataMode()->getNumberOfSpatialStreams())
                candidates.push_back(candidate);
        }
        // IEEE 802.11-2024 10.6.6.5.3: with no Basic HT-MCS Set modelled,
        // CandidateMCSSet uses the mandatory-index fallback. After the
        // bandwidth and MCS-index bounds,
        // retain the highest NSS not exceeding the received NSS, then select the
        // highest indexed MCS whose per-stream modulation and coding rate do not
        // exceed those of the received MCS. The modelled MCS 0..31 are EQM.
        int highestNss = -1;
        for (auto candidate : candidates)
            highestNss = std::max(highestNss, candidate->getDataMode()->getNumberOfSpatialStreams());
        const Ieee80211HtMode *response = nullptr;
        auto sourceMcs = source->getDataMode()->getModulationAndCodingScheme();
        int sourceModulation = sourceMcs->getModulation()->getSubcarrierModulation()->getCodeWordSize();
        double sourceCodeRate = sourceMcs->getCode()->getForwardErrorCorrection()->getCodeRate();
        for (auto candidate : candidates) {
            auto candidateDataMode = candidate->getDataMode();
            auto candidateMcs = candidateDataMode->getModulationAndCodingScheme();
            if (candidateDataMode->getNumberOfSpatialStreams() == highestNss &&
                candidateMcs->getModulation()->getSubcarrierModulation()->getCodeWordSize() <= sourceModulation &&
                candidateMcs->getCode()->getForwardErrorCorrection()->getCodeRate() <= sourceCodeRate &&
                (response == nullptr || candidateDataMode->getMcsIndex() > response->getDataMode()->getMcsIndex()))
                response = candidate;
        }
        if (response != nullptr)
            result.emplace(sourceEntry.mode, response);
    }
    return result;
}

std::map<const IIeee80211Mode *, const IIeee80211Mode *> Ieee80211ModeSet::createHtMixedControlResponseModes(const std::vector<Entry>& supportedEntries)
{
    std::map<const IIeee80211Mode *, const IIeee80211Mode *> result;
    for (const auto& sourceEntry : supportedEntries) {
        auto source = dynamic_cast<const Ieee80211HtMode *>(sourceEntry.mode);
        if (source == nullptr)
            continue;
        for (const auto& candidateEntry : supportedEntries) {
            auto candidate = dynamic_cast<const Ieee80211HtMode *>(candidateEntry.mode);
            if (candidate != nullptr && candidate->getPreambleMode()->getPreambleFormat() == Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED &&
                candidate->getCenterFrequencyMode() == source->getCenterFrequencyMode() &&
                candidate->getDataMode()->getMcsIndex() == source->getDataMode()->getMcsIndex() &&
                candidate->getDataMode()->getBandwidth() == source->getDataMode()->getBandwidth() &&
                candidate->getDataMode()->getNumberOfSpatialStreams() == source->getDataMode()->getNumberOfSpatialStreams() &&
                candidate->getDataMode()->getGuardIntervalType() == source->getDataMode()->getGuardIntervalType())
            {
                result.emplace(sourceEntry.mode, candidateEntry.mode);
                break;
            }
        }
    }
    return result;
}

std::vector<Ieee80211ModeSet::Entry> Ieee80211ModeSet::createNonHtControlResponseEntries(const std::vector<Entry>& supportedEntries)
{
    std::vector<Entry> result;
    for (const auto& entry : supportedEntries)
        if (entry.isMandatory && dynamic_cast<const Ieee80211HtMode *>(entry.mode) == nullptr && dynamic_cast<const Ieee80211VhtMode *>(entry.mode) == nullptr)
            result.push_back(entry);
    std::stable_sort(result.begin(), result.end(), EntryNetBitrateComparator());
    return result;
}

bool Ieee80211ModeSet::supportsMode(const IIeee80211Mode *mode) const
{
    for (const auto& entry : supportedEntries)
        if (entry.mode == mode)
            return true;
    return false;
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
    int index = findModeIndex(mode);
    if (index >= 0)
        return entries[index].isMandatory;
    for (const auto *legacyMode : legacyOperationalModes) {
        if (legacyMode == mode) {
            for (const auto& entry : supportedEntries) {
                if (entry.mode == mode)
                    return entry.isMandatory;
            }
        }
    }
    throw cRuntimeError("Unknown mode");
}

const IIeee80211Mode *Ieee80211ModeSet::findMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    return index >= 0 ? entries[index].mode : nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getMode(const IIeee80211Mode *mode) const
{
    auto result = findMode(mode);
    if (result == nullptr)
        throw cRuntimeError("Unknown mode in operation mode: '%s'", getName());
    return result;
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

const IIeee80211Mode *Ieee80211ModeSet::getFastestLegacyOperationalMode() const
{
    const IIeee80211Mode *fastestMandatoryLegacyMode = nullptr;
    for (const auto *mode : legacyOperationalModes) {
        if (getIsMandatory(mode))
            fastestMandatoryLegacyMode = mode;
    }
    if (fastestMandatoryLegacyMode != nullptr)
        return fastestMandatoryLegacyMode;
    return legacyOperationalModes.empty() ? nullptr : legacyOperationalModes.back();
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

const IIeee80211Mode *Ieee80211ModeSet::getControlResponseMode(const IIeee80211Mode *mode, const IIeee80211Mode *configuredMode) const
{
    if (!supportsMode(mode))
        throw cRuntimeError("Control response mode is not supported by operation mode %s: %s", getName(), mode->getName());
    auto it = controlResponseModes.find(mode);
    if (it == controlResponseModes.end()) {
        if (configuredMode != nullptr)
            return getNonHtControlResponseMode(configuredMode, false);
        return getMandatoryControlResponseMode(mode);
    }
    auto primaryMode = it->second;
    if (configuredMode == nullptr)
        return primaryMode;
    // A configured HT response is a deliberate model extension beyond the
    // automatic response constraints in IEEE 802.11-2024 10.6.6.5.3 and
    // 10.6.6.5.7. Translate only its preamble to HT-mixed, preserving the
    // resolved configured mode's MCS, bandwidth, NSS, GI, and band.
    if (!supportsMode(configuredMode))
        throw cRuntimeError("Configured control response mode is not supported by operation mode %s: %s", getName(), configuredMode->getName());
    auto configuredIt = htMixedControlResponseModes.find(configuredMode);
    if (configuredIt == htMixedControlResponseModes.end())
        throw cRuntimeError("An HT RTS requires an HT-mixed CTS response, configured mode is non-HT: %s", configuredMode->getName());
    return configuredIt->second;
}

const IIeee80211Mode *Ieee80211ModeSet::getMandatoryControlResponseMode(const IIeee80211Mode *mode) const
{
    if (!supportsMode(mode))
        throw cRuntimeError("Control response mode is not supported by operation mode %s: %s", getName(), mode->getName());
    if (!nonHtControlResponseEntries.empty() && (controlResponseModes.find(mode) != controlResponseModes.end() || !containsMode(mode)))
        return getNonHtControlResponseMode(mode);
    if (getIsMandatory(mode))
        return mode;
    if (auto slowerMode = getSlowerMandatoryMode(mode))
        return slowerMode;
    return getNonHtControlResponseMode(mode);
}

const IIeee80211Mode *Ieee80211ModeSet::getNonHtControlResponseMode(const IIeee80211Mode *mode, bool mandatory) const
{
    if (!supportsMode(mode))
        throw cRuntimeError("Control response mode is not supported by operation mode %s: %s", getName(), mode->getName());
    if (nonHtControlResponseEntries.empty()) {
        if (!containsMode(mode))
            throw cRuntimeError("No non-HT control response mode for %s", mode->getName());
        if (!mandatory)
            return mode;
        if (getIsMandatory(mode))
            return mode;
        if (auto slowerMode = getSlowerMandatoryMode(mode))
            return slowerMode;
        throw cRuntimeError("No mandatory control response mode for %s", mode->getName());
    }
    if (controlResponseModes.find(mode) == controlResponseModes.end()) {
        // VHT response-format selection remains unchanged; this fallback is HT-scoped.
        if (dynamic_cast<const Ieee80211VhtMode *>(mode) != nullptr)
            return mode;
        if (!mandatory)
            return mode;
    }
    const IIeee80211Mode *result = nullptr;
    for (const auto& entry : nonHtControlResponseEntries) {
        auto candidate = entry.mode;
        if (candidate->getDataMode()->getNetBitrate() <= mode->getDataMode()->getNetBitrate() &&
            (result == nullptr || candidate->getDataMode()->getNetBitrate() > result->getDataMode()->getNetBitrate()))
            result = candidate;
    }
    if (result == nullptr)
        throw cRuntimeError("No mandatory non-HT control response mode for %s", mode->getName());
    return result;
}

const IIeee80211Mode *Ieee80211ModeSet::findHtMixedMode(const IIeee80211Mode *mode) const
{
    if (mode == nullptr)
        return nullptr;
    auto it = htMixedControlResponseModes.find(mode);
    return it != htMixedControlResponseModes.end() ? it->second : nullptr;
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
