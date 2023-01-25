//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"

namespace inet {
namespace physicallayer {

SignalPacketModel::SignalPacketModel(const Packet *packet, bps headerBitrate, bps dataBitrate) :
    packet(packet),
    headerBitrate(headerBitrate),
    dataBitrate(dataBitrate)
{
}

std::ostream& SignalPacketModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SignalPacketModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(packet);
    return stream;
}

TransmissionPacketModel::TransmissionPacketModel(const Packet *packet, bps headerBitrate, bps dataBitrate) :
    SignalPacketModel(packet, headerBitrate, dataBitrate)
{
}

ReceptionPacketModel::ReceptionPacketModel(const Packet *packet, bps headerBitrate, bps dataBitrate) :
    SignalPacketModel(packet, headerBitrate, dataBitrate)
{
}


} // namespace physicallayer
} // namespace inet

