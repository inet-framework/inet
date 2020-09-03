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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace physicallayer {

NarrowbandTransmitterBase::NarrowbandTransmitterBase() :
    modulation(nullptr),
    centerFrequency(Hz(NaN)),
    bandwidth(Hz(NaN))
{
}

void NarrowbandTransmitterBase::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        modulation = ApskModulationBase::findModulation(par("modulation"));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}

std::ostream& NarrowbandTransmitterBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modulation, printFieldToString(modulation, level + 1, evFlags))
               << EV_FIELD(centerFrequency)
               << EV_FIELD(bandwidth);
    return stream;
}


Hz NarrowbandTransmitterBase::computeCenterFrequency(const Packet *packet) const
{
    const auto& signalBandReq = const_cast<Packet *>(packet)->findTag<SignalBandReq>();
    return signalBandReq != nullptr ? signalBandReq->getCenterFrequency() : centerFrequency;
}

Hz NarrowbandTransmitterBase::computeBandwidth(const Packet *packet) const
{
    const auto& signalBandReq = const_cast<Packet *>(packet)->findTag<SignalBandReq>();
    return signalBandReq != nullptr ? signalBandReq->getBandwidth() : bandwidth;
}

} // namespace physicallayer
} // namespace inet

