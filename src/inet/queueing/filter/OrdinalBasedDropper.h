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

#ifndef __INET_ORDINALBASEDDROPPER_H
#define __INET_ORDINALBASEDDROPPER_H

#include <vector>
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {
namespace queueing {

/**
 * Ordinal Based Dropper module.
 */
class INET_API OrdinalBasedDropper : public PacketFilterBase
{
  protected:
    unsigned int numPackets;
    unsigned int numDropped;

    bool generateFurtherDrops;
    std::vector<unsigned int> dropsVector;

  protected:
    virtual void initialize(int stage) override;
    virtual void parseVector(const char *vector);

    virtual void processPacket(Packet *packet) override;
    virtual void dropPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_ORDINALBASEDDROPPER_H

