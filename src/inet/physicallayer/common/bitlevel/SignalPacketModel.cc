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

SignalPacketModel::SignalPacketModel(const Packet *packet, bps bitrate) :
    packet(packet),
    bitrate(bitrate)
{
}

std::ostream& SignalPacketModel::printToStream(std::ostream& stream, int level) const
{
    stream << "SignalPacketModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", packet = " << packet;
    return stream;
}

TransmissionPacketModel::TransmissionPacketModel(const Packet *packet, bps bitrate) :
    SignalPacketModel(packet, bitrate)
{
}

ReceptionPacketModel::ReceptionPacketModel(const Packet *packet, bps bitrate) :
    SignalPacketModel(packet, bitrate)
{
}

} // namespace physicallayer
} // namespace inet

