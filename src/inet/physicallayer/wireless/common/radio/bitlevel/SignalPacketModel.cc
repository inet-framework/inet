//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

