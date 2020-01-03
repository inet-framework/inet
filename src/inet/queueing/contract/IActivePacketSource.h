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

#ifndef __INET_IACTIVEPACKETSOURCE_H
#define __INET_IACTIVEPACKETSOURCE_H

#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for active packet sources.
 */
class INET_API IActivePacketSource
{
  public:
    virtual ~IActivePacketSource() {}

    /**
     * Returns the passive packet sink where packets are pushed. The gate must
     * not be nullptr.
     */
    virtual IPassivePacketSink *getConsumer(cGate *gate) = 0;

    /**
     * Notifies about a state change that allows to push some packet into the
     * passive packet sink at the given gate. The gate is never nullptr.
     */
    virtual void handleCanPushPacket(cGate *gate) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IACTIVEPACKETSOURCE_H

