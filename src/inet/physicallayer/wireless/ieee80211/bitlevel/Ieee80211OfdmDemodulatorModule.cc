//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDemodulatorModule.h"

#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OfdmDemodulatorModule);

void Ieee80211OfdmDemodulatorModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        ofdmDemodulator = new Ieee80211OfdmDemodulator(new Ieee80211OfdmModulation(par("numSubcarriers"), ApskModulationBase::findModulation(par("subcarrierModulation"))));
}

const IReceptionBitModel *Ieee80211OfdmDemodulatorModule::demodulate(const IReceptionSymbolModel *symbolModel) const
{
    return ofdmDemodulator->demodulate(symbolModel);
}

Ieee80211OfdmDemodulatorModule::~Ieee80211OfdmDemodulatorModule()
{
    delete ofdmDemodulator->getModulation();
    delete ofdmDemodulator;
}

} /* namespace physicallayer */
} /* namespace inet */

