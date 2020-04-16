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

#ifndef __INET_IPACKETCOMPARATORFUNCTION_H
#define __INET_IPACKETCOMPARATORFUNCTION_H

#include "inet/common/packet/Packet.h"
#include "inet/queueing/compat/cqueue.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet comparator functions.
 */
class INET_API IPacketComparatorFunction : public cQueue::Comparator
{
  public:
    virtual ~IPacketComparatorFunction() {}

    /**
     * Returns the order of the given packets.
     */
    virtual int comparePackets(Packet *packet1, Packet *packet2) const = 0;

    // Implements cQueue::Comparator interface
    virtual bool less(cObject *a, cObject *b) override { return comparePackets(check_and_cast<Packet *>(a), check_and_cast<Packet *>(b)) < 0; }
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPACKETCOMPARATORFUNCTION_H

