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

#include "inet/queueing/marker/PacketTagger.h"

namespace inet {
namespace queueing {

Define_Module(PacketTagger);

PacketTagger::~PacketTagger()
{
    delete packetFilterFunction;
}

void PacketTagger::initialize(int stage)
{
    PacketTaggerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        packetFilterFunction = createFilterFunction(par("filterClass"));
}

IPacketFilterFunction *PacketTagger::createFilterFunction(const char *filterClass) const
{
    return check_and_cast<IPacketFilterFunction *>(createOne(filterClass));
}

void PacketTagger::markPacket(Packet *packet)
{
    if (packetFilterFunction->matchesPacket(packet))
        PacketTaggerBase::markPacket(packet);
}

} // namespace queueing
} // namespace inet

