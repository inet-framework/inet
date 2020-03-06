//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace physicallayer {

FlatTransmitterBase::FlatTransmitterBase() :
    NarrowbandTransmitterBase(),
    preambleDuration(-1),
    headerLength(b(-1)),
    bitrate(bps(NaN)),
    codeRate(NaN),
    power(W(NaN))
{
}

void FlatTransmitterBase::initialize(int stage)
{
    NarrowbandTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        preambleDuration = par("preambleDuration");
        headerLength = b(par("headerLength"));
        bitrate = bps(par("bitrate"));
        codeRate = par("codeRate");
        power = W(par("power"));
    }
}

std::ostream& FlatTransmitterBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(preambleDuration)
               << EV_FIELD(headerLength)
               << EV_FIELD(bitrate)
               << EV_FIELD(codeRate)
               << EV_FIELD(power);
    return NarrowbandTransmitterBase::printToStream(stream, level);
}

bps FlatTransmitterBase::computeTransmissionPreambleBitrate(const Packet *packet) const
{
    const auto& signalBitrateReq = const_cast<Packet *>(packet)->findTag<SignalBitrateReq>();
    return signalBitrateReq != nullptr ? signalBitrateReq->getPreambleBitrate() : bitrate;
}

bps FlatTransmitterBase::computeTransmissionHeaderBitrate(const Packet *packet) const
{
    const auto& signalBitrateReq = const_cast<Packet *>(packet)->findTag<SignalBitrateReq>();
    return signalBitrateReq != nullptr ? signalBitrateReq->getHeaderBitrate() : bitrate;
}

bps FlatTransmitterBase::computeTransmissionDataBitrate(const Packet *packet) const
{
    const auto& signalBitrateReq = const_cast<Packet *>(packet)->findTag<SignalBitrateReq>();
    return signalBitrateReq != nullptr ? signalBitrateReq->getDataBitrate() : bitrate;
}

W FlatTransmitterBase::computeTransmissionPower(const Packet *packet) const
{
    const auto& signalPowerReq = const_cast<Packet *>(packet)->findTag<SignalPowerReq>();
    return signalPowerReq != nullptr ? signalPowerReq->getPower() : power;
}

} // namespace physicallayer
} // namespace inet

