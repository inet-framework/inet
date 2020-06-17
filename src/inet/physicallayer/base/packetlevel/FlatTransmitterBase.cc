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

#include "inet/physicallayer/base/packetlevel/FlatTransmitterBase.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace physicallayer {

FlatTransmitterBase::FlatTransmitterBase() :
    NarrowbandTransmitterBase(),
    preambleDuration(-1),
    headerLength(b(-1)),
    bitrate(bps(NaN)),
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
        power = W(par("power"));
    }
}

std::ostream& FlatTransmitterBase::printToStream(std::ostream& stream, int level) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", preambleDuration = " << preambleDuration
               << ", headerLength = " << headerLength
               << ", bitrate = " << bitrate
               << ", power = " << power;
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

