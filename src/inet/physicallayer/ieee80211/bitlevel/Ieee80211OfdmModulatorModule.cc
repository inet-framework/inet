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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmModulatorModule.h"
#include "inet/physicallayer/modulation/BpskModulation.h"
#include "inet/physicallayer/modulation/Qam16Modulation.h"
#include "inet/physicallayer/modulation/Qam64Modulation.h"
#include "inet/physicallayer/modulation/QpskModulation.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211OfdmModulatorModule);

void Ieee80211OfdmModulatorModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        ofdmModulator = new Ieee80211OfdmModulator(new Ieee80211OfdmModulation(ApskModulationBase::findModulation(par("subcarrierModulation"))), par("pilotSubcarrierPolarityVectorOffset"));
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

