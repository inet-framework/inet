//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskPhyHeader_m.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmission.h"

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

std::ostream& UnitDiskTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskTransmitter";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(preambleDuration)
               << EV_FIELD(headerLength)
               << EV_FIELD(bitrate);
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(communicationRange);
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(interferenceRange)
               << EV_FIELD(detectionRange);
    return stream;
}

const ITransmission *UnitDiskTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    auto phyHeader = packet->peekAtFront<UnitDiskPhyHeader>();
    auto dataLength = packet->getTotalLength() - phyHeader->getChunkLength();
    const auto& signalBitrateReq = const_cast<Packet *>(packet)->findTag<SignalBitrateReq>();
    auto transmissionBitrate = signalBitrateReq != nullptr ? signalBitrateReq->getDataBitrate() : bitrate;
    if (!(transmissionBitrate > bps(0)))
        throw cRuntimeError("Missing transmission bitrate (got %g): No bitrate request on packet, and bitrate parameter not set", transmissionBitrate.get());
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

