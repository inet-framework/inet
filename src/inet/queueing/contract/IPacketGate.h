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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IPACKETGATE_H
#define __INET_IPACKETGATE_H

#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet gates.
 */
class INET_API IPacketGate
{
  public:
    /**
     * Returns true if the gate is open.
     */
    virtual bool isOpen() const = 0;

    /**
     * Opens the gate and starts traffic go through.
     */
    virtual void open() = 0;

    /**
     * Closes the gate and stops traffic.
     */
    virtual void close() = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPACKETGATE_H

