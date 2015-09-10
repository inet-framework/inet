//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "Ieee80211OFDMModulatorModule.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OFDMModulatorModule);

void Ieee80211OFDMModulatorModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        ofdmModulator = new Ieee80211OFDMModulator(new Ieee80211OFDMModulation(APSKModulationBase::findModulation(par("subcarrierModulation"))), par("pilotSubcarrierPolarityVectorOffset"));
}

const ITransmissionSymbolModel *Ieee80211OFDMModulatorModule::modulate(const ITransmissionBitModel *bitModel) const
{
    return ofdmModulator->modulate(bitModel);
}

Ieee80211OFDMModulatorModule::~Ieee80211OFDMModulatorModule()
{
    delete ofdmModulator;
}
} /* namespace physicallayer */
} /* namespace inet */

