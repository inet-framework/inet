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

#include "inet/common/packet/ByteCountChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKPhyHeader_m.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKRadio.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKRadio);

APSKRadio::APSKRadio() :
    FlatRadioBase()
{
}

void APSKRadio::encapsulate(Packet *packet) const
{
    auto flatTransmitter = check_and_cast<const FlatTransmitterBase *>(transmitter);
    auto phyHeader = std::make_shared<APSKPhyHeader>();
    phyHeader->setChunkLength((flatTransmitter->getHeaderBitLength() + 7) / 8);
    phyHeader->markImmutable();
    packet->pushHeader(phyHeader);
}

void APSKRadio::decapsulate(Packet *packet) const
{
    packet->popHeader<APSKPhyHeader>();
}

} // namespace physicallayer

} // namespace inet

