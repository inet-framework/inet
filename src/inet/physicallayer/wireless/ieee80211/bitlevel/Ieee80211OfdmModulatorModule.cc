//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmModulatorModule.h"

#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OfdmModulatorModule);

void Ieee80211OfdmModulatorModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        ofdmModulator = new Ieee80211OfdmModulator(new Ieee80211OfdmModulation(par("numSubcarriers"), ApskModulationBase::findModulation(par("subcarrierModulation"))), par("pilotSubcarrierPolarityVectorOffset"));
}

const ITransmissionSymbolModel *Ieee80211OfdmModulatorModule::modulate(const ITransmissionBitModel *bitModel) const
{
    return ofdmModulator->modulate(bitModel);
}

Ieee80211OfdmModulatorModule::~Ieee80211OfdmModulatorModule()
{
    delete ofdmModulator;
}

} /* namespace physicallayer */
} /* namespace inet */

