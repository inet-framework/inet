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

#ifndef __INET_IACTIVEPACKETSINK_H
#define __INET_IACTIVEPACKETSINK_H

#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for active packet sinks.
 */
class INET_API IActivePacketSink
{
  public:
    virtual ~IActivePacketSink() {}

    /**
     * Returns the passive packet source from where packets are collected. The
     * gate must not be nullptr.
     */
    virtual IPassivePacketSource *getProvider(cGate *gate) = 0;

    /**
     * Notifies about a state change that allows to pull some packet from the
     * passive packet source at the given gate. The gate is never nulltr.
     */
    virtual void handleCanPullPacket(cGate *gate) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IACTIVEPACKETSINK_H

