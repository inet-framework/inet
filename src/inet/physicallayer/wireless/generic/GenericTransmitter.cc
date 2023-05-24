//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/generic/GenericTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/generic/GenericPhyHeader_m.h"
#include "inet/physicallayer/wireless/generic/GenericTransmission.h"

namespace inet {
namespace physicallayer {

Define_Module(GenericTransmitter);

GenericTransmitter::GenericTransmitter() :
    headerLength(b(-1)),
    bitrate(NaN)
{
}

void GenericTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        preambleDuration = par("preambleDuration");
        headerLength = b(par("headerLength"));
        bitrate = bps(par("bitrate"));
    }
}

std::ostream& GenericTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "GenericTransmitter";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(preambleDuration)
               << EV_FIELD(headerLength)
               << EV_FIELD(bitrate);
    return stream;
}

const ITransmission *GenericTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    auto phyHeader = packet->peekAtFront<GenericPhyHeader>();
    auto dataLength = packet->getDataLength() - phyHeader->getChunkLength();
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
    auto transmission = new GenericTransmission(transmitter, packet, startTime, endTime,
            preambleDuration, headerDuration, dataDuration,
            startPosition, endPosition, startOrientation, endOrientation);
    transmission->newAnalogModel = getAnalogModel()->createAnalogModel(packet, duration, Hz(NaN), Hz(NaN), W(NaN));
    return transmission;
}

} // namespace physicallayer
} // namespace inet

