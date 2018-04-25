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

#ifndef __INET_QAM1024MODULATION_H
#define __INET_QAM1024MODULATION_H

#include "inet/physicallayer/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements gray coded rectangular quadrature amplitude
 * modulation with 256 symbols.
 *
 * http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation
 */
class INET_API Qam1024Modulation : public MqamModulationBase
{
  public:
    static const Qam1024Modulation singleton;

  protected:
    static const std::vector<ApskSymbol> constellation;

  public:
    Qam1024Modulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Qam1024Modulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_QAM1024MODULATION_H

