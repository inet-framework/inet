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

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/unitdisk/UnitDiskPhyHeader_m.h"
#include "inet/physicallayer/unitdisk/UnitDiskTransmission.h"
#include "inet/physicallayer/unitdisk/UnitDiskTransmitter.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskTransmitter);

UnitDiskTransmitter::UnitDiskTransmitter() :
    headerLength(b(-1)),
    bitrate(NaN),
    communicationRange(NaN),
    interferenceRange(NaN),
    detectionRange(NaN)
{
}

void UnitDiskTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        preambleDuration = par("preambleDuration");
        headerLength = b(par("headerLength"));
        bitrate = bps(par("bitrate"));
        communicationRange = m(par("communicationRange"));
        interferenceRange = m(par("interferenceRange"));
        detectionRange = m(par("detectionRange"));
    }
}

std::ostream& UnitDiskTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "UnitDiskTransmitter";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", preambleDuration = " << preambleDuration
               << ", headerLength = " << headerLength
               << ", bitrate = " << bitrate;
    if (level <= PRINT_LEVEL_INFO)
        stream << ", communicationRange = " << communicationRange;
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", interferenceRange = " << interferenceRange
               << ", detectionRange = " << detectionRange;
    return stream;
}

const ITransmission *UnitDiskTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    auto phyHeader = packet->peekAtFront<UnitDiskPhyHeader>();
    auto dataLength = packet->getTotalLength() - phyHeader->getChunkLength();
    auto signalBitrateReq = const_cast<Packet *>(packet)->findTag<SignalBitrateReq>();
    auto transmissionBitrate = signalBitrateReq != nullptr ? signalBitrateReq->getDataBitrate() : bitrate;
    auto headerDuration = b(headerLength).get() / bps(transmissionBitrate).get();
    auto dataDuration = b(dataLength).get() / bps(transmissionBitrate).get();
    auto duration = preambleDuration + headerDuration + dataDuration;
    auto endTime = startTime + duration;
    auto mobility = transmitter->getAntenna()->getMobility();
    auto startPosition = mobility->getCurrentPosition();
    auto endPosition = mobility->getCurrentPosition();
    auto startOrientation = mobility->getCurrentAngularPosition();
    auto endOrientation = mobility->getCurrentAngularPosition();
    return new UnitDiskTransmission(transmitter, packet, startTime, endTime,
            preambleDuration, headerDuration, dataDuration,
            startPosition, endPosition, startOrientation, endOrientation,
            communicationRange, interferenceRange, detectionRange);
}

} // namespace physicallayer
} // namespace inet

