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

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/idealradio/IdealPhyHeader_m.h"
#include "inet/physicallayer/idealradio/UnitDiskRadio.h"
#include "inet/physicallayer/idealradio/IdealTransmitter.h"

namespace inet {

namespace physicallayer {

Define_Module(UnitDiskRadio);

UnitDiskRadio::UnitDiskRadio() :
    Radio()
{
}

void UnitDiskRadio::encapsulate(Packet *packet) const
{
    auto idealTransmitter = check_and_cast<const IdealTransmitter *>(transmitter);
    auto phyHeader = makeShared<IdealPhyHeader>();
    phyHeader->setChunkLength(idealTransmitter->getHeaderLength());
    packet->pushHeader(phyHeader);
}

void UnitDiskRadio::decapsulate(Packet *packet) const
{
    packet->popHeader<IdealPhyHeader>();
}

} // namespace physicallayer

} // namespace inet

