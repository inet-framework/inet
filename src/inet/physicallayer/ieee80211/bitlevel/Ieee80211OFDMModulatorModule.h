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

#ifndef __INET_IEEE80211OFDMMODULATORMODULE_H
#define __INET_IEEE80211OFDMMODULATORMODULE_H

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmModulator.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmModulatorModule : public IModulator, public cSimpleModule
{
  protected:
    const Ieee80211OfdmModulator *ofdmModulator;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~Ieee80211OfdmModulatorModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return ofdmModulator->printToStream(stream, level); }
    const Ieee80211OfdmModulation *getModulation() const override { return ofdmModulator->getModulation(); }
    const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211OFDMMODULATORMODULE_H

