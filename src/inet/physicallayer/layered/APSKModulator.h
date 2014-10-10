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

#ifndef __INET_APSKMODULATOR_H
#define __INET_APSKMODULATOR_H

#include "inet/physicallayer/contract/IModulator.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/contract/ISignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API APSKModulator : public cSimpleModule, public IModulator
{
  protected:
    int preambleSymbolLength;
    const APSKModulationBase *modulationScheme;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg) { throw cRuntimeError("The module doesn't handle self messages"); }

  public:
    APSKModulator() : modulationScheme(NULL) {};
    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const;
    void printToStream(std::ostream& stream) const { stream << "TODO"; }
};

} // namespace physicallayer

} // namespace inet

#endif /* __INET_APSKMODULATOR_H */
