//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredTransmission.h"

namespace inet {
namespace physicallayer {

LayeredTransmission::LayeredTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation) :
    TransmissionBase(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation),
    packetModel(packetModel),
    bitModel(bitModel),
    symbolModel(symbolModel),
    sampleModel(sampleModel),
    analogModel(analogModel)
{
}

LayeredTransmission::~LayeredTransmission()
{
    delete packetModel;
    delete bitModel;
    delete symbolModel;
    delete sampleModel;
    delete analogModel;
}

std::ostream& LayeredTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LayeredTransmission";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(packetModel, printFieldToString(packetModel, level + 1, evFlags))
               << EV_FIELD(bitModel, printFieldToString(bitModel, level + 1, evFlags))
               << EV_FIELD(symbolModel, printFieldToString(symbolModel, level + 1, evFlags))
               << EV_FIELD(sampleModel, printFieldToString(sampleModel, level + 1, evFlags))
               << EV_FIELD(analogModel, printFieldToString(analogModel, level + 1, evFlags));
    return TransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer
} // namespace inet

