//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"

namespace inet {
namespace physicallayer {

SignalPacketModel::SignalPacketModel(const Packet *packet, bps bitrate) :
    packet(packet),
    bitrate(bitrate)
{
}

std::ostream& SignalPacketModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SignalPacketModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(packet);
    return stream;
}

TransmissionPacketModel::TransmissionPacketModel(const Packet *packet, bps bitrate) :
    SignalPacketModel(packet, bitrate)
{
}

ReceptionPacketModel::ReceptionPacketModel(const Packet *packet, bps bitrate, double packetErrorRate) :
    SignalPacketModel(packet, bitrate),
    packetErrorRate(packetErrorRate)
{
}

} // namespace physicallayer
} // namespace inet

