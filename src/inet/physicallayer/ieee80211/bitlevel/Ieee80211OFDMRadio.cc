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

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211LayeredOFDMTransmitter.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMRadio.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OFDMRadio);

Ieee80211OFDMRadio::Ieee80211OFDMRadio() :
    FlatRadioBase()
{
}

void Ieee80211OFDMRadio::encapsulate(Packet *packet) const
{
    auto ofdmTransmitter = check_and_cast<const Ieee80211LayeredOFDMTransmitter *>(transmitter);
    auto paddingLength = ofdmTransmitter->getPaddingLength(packet);
    const auto& phyHeader = makeShared<Ieee80211OfdmPhyHeader>();
    phyHeader->setRate(ofdmTransmitter->getMode(packet)->getSignalMode()->getRate());
    phyHeader->setLengthField(B(packet->getTotalLength()).get());
    packet->insertHeader(phyHeader);
    if (paddingLength != b(0)) {
        const auto& phyTrailer = makeShared<BitCountChunk>(paddingLength);
        packet->insertTrailer(phyTrailer);
    }
}

void Ieee80211OFDMRadio::decapsulate(Packet *packet) const
{
    packet->popHeader<Ieee80211OfdmPhyHeader>();
    packet->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211);
}

} // namespace physicallayer

} // namespace inet

