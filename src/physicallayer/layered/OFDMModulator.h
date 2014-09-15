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

#ifndef __INET_OFDMMODULATOR_H
#define __INET_OFDMMODULATOR_H

#include "IModulator.h"
#include "IModulationScheme.h"
#include "SignalBitModel.h"
#include "SignalSymbolModel.h"

namespace inet {

namespace physicallayer {

class INET_API OFDMModulator : public IModulator
{
  protected:
    int preambleSymbolLength;
    const IModulationScheme *modulationScheme;

  public:
    OFDMModulator();
    OFDMModulator(const char *modulationScheme);
    virtual const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const;
    double calculateBER(double snir, double bandwidth, double bitrate) const { return 42; } // TODO: (Modulator, ModulationScheme) -> BER
    const IModulationScheme *getModulation() const { return modulationScheme; }
    void printToStream(std::ostream& stream) const { stream << "TODO"; }
};

} // namespace physicallayer

} // namespace inet

#endif /* __INET_OFDMMODULATOR_H */
