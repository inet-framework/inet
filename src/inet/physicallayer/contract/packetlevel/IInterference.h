//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IINTERFERENCE_H
#define __INET_IINTERFERENCE_H

#include "inet/physicallayer/contract/packetlevel/INoise.h"
#include "inet/physicallayer/contract/packetlevel/IReception.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the interference related to a reception.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IInterference : public IPrintableObject
{
  public:
    virtual const INoise *getBackgroundNoise() const = 0;

    virtual const std::vector<const IReception *> *getInterferingReceptions() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IINTERFERENCE_H

