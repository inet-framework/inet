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

#ifndef __INET_ILISTENING_H
#define __INET_ILISTENING_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {

namespace physicallayer {

class IRadio;

/**
 * This interface represents how a receiver is listening on the radio channel.
 */
class INET_API IListening : public IPrintableObject
{
  public:
    virtual const IRadio *getReceiver() const = 0;

    virtual const simtime_t getStartTime() const = 0;
    virtual const simtime_t getEndTime() const = 0;

    virtual const Coord getStartPosition() const = 0;
    virtual const Coord getEndPosition() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ILISTENING_H

