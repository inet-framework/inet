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

#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"

namespace inet {

namespace physicallayer {

SignalPacketModel::SignalPacketModel(const cPacket *packet, const BitVector *serializedPacket, bps bitrate) :
    packet(packet),
    serializedPacket(serializedPacket),
    bitrate(bitrate)
{
}

SignalPacketModel::~SignalPacketModel()
{
    delete serializedPacket;
}

std::ostream& SignalPacketModel::printToStream(std::ostream& stream, int level) const
{
    stream << "SignalPacketModel";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", packet = " << packet;
    return stream;
}

TransmissionPacketModel::TransmissionPacketModel(const cPacket *packet, const BitVector *serializedPacket, bps bitrate) :
    SignalPacketModel(packet, serializedPacket, bitrate)
{
}

ReceptionPacketModel::ReceptionPacketModel(const cPacket *packet, const BitVector *serializedPacket, bps bitrate, double per, bool packetErrorless) :
    SignalPacketModel(packet, serializedPacket, bitrate),
    per(per),
    packetErrorless(packetErrorless)
{
}

} // namespace physicallayer

} // namespace inet

