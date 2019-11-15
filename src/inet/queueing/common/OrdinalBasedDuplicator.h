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

#ifndef __INET_ORDINALBASEDDUPLICATOR_H
#define __INET_ORDINALBASEDDUPLICATOR_H

#include <vector>
#include "inet/queueing/base/PacketDuplicatorBase.h"

namespace inet {
namespace queueing {

/**
 * Ordinal Based Duplicator module.
 */
class INET_API OrdinalBasedDuplicator : public PacketDuplicatorBase
{
  protected:
    unsigned int numPackets;
    unsigned int numDuplicated;
    bool generateFurtherDuplicates;

    std::vector<unsigned int> duplicatesVector;

  protected:
    virtual void initialize(int stage) override;
    virtual void parseVector(const char *vector);
    int getNumPacketDuplicates(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_ORDINALBASEDDUPLICATOR_H

