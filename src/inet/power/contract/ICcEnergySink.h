//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_ICCENERGYSINK_H
#define __INET_ICCENERGYSINK_H

#include "inet/power/contract/IEnergySink.h"

namespace inet {

namespace power {

/**
 * TODO
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API ICcEnergySink : public virtual IEnergySink
{
  public:
    /**
     * The signal that is used to publish current generation changes.
     */
    static simsignal_t currentGenerationChangedSignal;

  public:
    /**
     * Returns the total current generation in the range [0, +infinity).
     */
    virtual A getTotalCurrentGeneration() const = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ICCENERGYSINK_H

