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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmDemodulatorModule.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"
#include "inet/physicallayer/modulation/BpskModulation.h"
#include "inet/physicallayer/modulation/Qam16Modulation.h"
#include "inet/physicallayer/modulation/Qam64Modulation.h"
#include "inet/physicallayer/modulation/QpskModulation.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OfdmDemodulatorModule);

void Ieee80211OfdmDemodulatorModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        ofdmDemodulator = new Ieee80211OfdmDemodulator(new Ieee80211OfdmModulation(ApskModulationBase::findModulation(par("subcarrierModulation"))));
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

