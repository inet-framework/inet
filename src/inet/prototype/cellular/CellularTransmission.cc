//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/CellularTransmission.h"

namespace inet {

namespace cellular {

CellularTransmission::CellularTransmission(const IRadio *transmitterRadio, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const Coord& position, const Quaternion& orientation, const ITransmissionAnalogModel *analogModel, const Ptr<const IAntennaGain>& beamGain, const CarrierGrid& grid, const std::vector<int>& prbs, int numSymbols, int mcs, int intendedReceiverId) :
    TransmissionBase(transmitterRadio, packet, startTime, endTime, 0, 0, endTime - startTime, position, position, orientation, orientation, nullptr, nullptr, nullptr, nullptr, analogModel),
    grid(grid), prbs(prbs), numSymbols(numSymbols), mcs(mcs), intendedReceiverId(intendedReceiverId)
{
    // workaround for C1 (beam-addressable antenna): the base class takes the current gain of
    // the antenna, which is the element pattern, not the beam of this allocation
    transmitterGain = beamGain;
}

std::ostream& CellularTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "CellularTransmission";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(numPrbs, prbs.size())
               << EV_FIELD(numSymbols)
               << EV_FIELD(mcs)
               << EV_FIELD(intendedReceiverId);
    return TransmissionBase::printToStream(stream, level);
}

} // namespace cellular

} // namespace inet

