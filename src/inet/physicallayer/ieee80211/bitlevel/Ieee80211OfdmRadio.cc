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
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211LayeredOfdmTransmitter.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmRadio.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OfdmRadio);

Ieee80211OfdmRadio::Ieee80211OfdmRadio() :
    FlatRadioBase()
{
}

void Ieee80211OfdmRadio::encapsulate(Packet *packet) const
{
    auto ofdmTransmitter = check_and_cast<const Ieee80211LayeredOfdmTransmitter *>(transmitter);
    const auto& phyHeader = makeShared<Ieee80211OfdmPhyHeader>();
    phyHeader->setRate(ofdmTransmitter->getMode(packet)->getSignalMode()->getRate());
    phyHeader->setLengthField(B(packet->getTotalLength()));
    packet->insertAtFront(phyHeader);
    auto paddingLength = ofdmTransmitter->getPaddingLength(ofdmTransmitter->getMode(packet), B(phyHeader->getLengthField()));
    // insert padding and 6 tail bits
    const auto &phyTrailer = makeShared<BitCountChunk>(paddingLength + b(6));
    packet->insertAtBack(phyTrailer);
    packet->getTagForUpdate<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211OfdmPhy);
}

void Ieee80211OfdmRadio::decapsulate(Packet *packet) const
{
    auto ofdmTransmitter = check_and_cast<const Ieee80211LayeredOfdmTransmitter *>(transmitter);
    const auto& phyHeader = packet->popAtFront<Ieee80211OfdmPhyHeader>();
    auto paddingLength = ofdmTransmitter->getPaddingLength(ofdmTransmitter->getMode(packet), B(phyHeader->getLengthField()));
    // pop padding and 6 tail bits
    packet->popAtBack(paddingLength + b(6));
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
}

} // namespace physicallayer

} // namespace inet

