//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmitterBase.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

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

