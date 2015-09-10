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

#ifndef __INET_MQAMMODULATIONBASE_H
#define __INET_MQAMMODULATIONBASE_H

#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * Base class for gray coded rectangular quadrature amplitude modulations.
 */
class INET_API MQAMModulationBase : public APSKModulationBase
{
  public:
    MQAMModulationBase(const std::vector<APSKSymbol> *constellation);

    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const;
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MQAMMODULATIONBASE_H

