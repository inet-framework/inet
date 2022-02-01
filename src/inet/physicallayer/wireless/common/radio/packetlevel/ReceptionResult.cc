//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionResult.h"

namespace inet {
namespace physicallayer {

ReceptionResult::ReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const Packet *packet) :
    reception(reception),
    decisions(decisions),
    packet(packet)
{
}

ReceptionResult::~ReceptionResult()
{
    delete packet;
    delete decisions;
}

std::ostream& ReceptionResult::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ReceptionResult";
    return stream;
}

const Packet *ReceptionResult::getPacket() const
{
    return packet;
}

} // namespace physicallayer
} // namespace inet

