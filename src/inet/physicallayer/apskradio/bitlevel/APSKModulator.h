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

#include "inet/physicallayer/contract/layered/IModulator.h"
#include "inet/physicallayer/common/layered/SignalBitModel.h"
#include "inet/physicallayer/common/layered/SignalSymbolModel.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/apsk/layered/APSKSymbol.h"

namespace inet {

namespace physicallayer {

class INET_API APSKModulator : public IModulator, public cSimpleModule
{
  protected:
    const APSKModulationBase *modulation;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);

  public:
    APSKModulator();

    virtual void printToStream(std::ostream& stream) const { stream << "APSKModulator"; }
    virtual const IModulation *getModulation() const { return modulation; }
    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKMODULATOR_H

