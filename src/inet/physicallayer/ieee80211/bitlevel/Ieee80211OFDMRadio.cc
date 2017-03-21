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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMPLCPFrame_m.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMRadio.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OFDMRadio);

Ieee80211OFDMRadio::Ieee80211OFDMRadio() :
    FlatRadioBase()
{
}

void Ieee80211OFDMRadio::encapsulate(Packet *packet) const
{
    // The PLCP header is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1),
    // Tail (6) and SERVICE (16) fields.
    int plcpHeaderLength = 4 + 1 + 12 + 1 + 6 + 16;
    auto phyHeader = std::make_shared<Ieee80211OFDMPLCPFrame>();
    phyHeader->setChunkLength(bit(plcpHeaderLength));
// TODO: phyHeader->setRate(mode->getSignalMode()->getRate());
    phyHeader->setLength(byte(packet->getPacketLength()).get());
    phyHeader->markImmutable();
    packet->pushHeader(phyHeader);
}

void Ieee80211OFDMRadio::decapsulate(Packet *packet) const
{
    packet->popHeader<Ieee80211OFDMPLCPFrame>();
}

} // namespace physicallayer

} // namespace inet

