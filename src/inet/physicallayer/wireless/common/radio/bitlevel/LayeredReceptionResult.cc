//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/LayeredReceptionResult.h"

namespace inet {
namespace physicallayer {

LayeredReceptionResult::LayeredReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const IReceptionPacketModel *packetModel, const IReceptionBitModel *bitModel, const IReceptionSymbolModel *symbolModel, const IReceptionSampleModel *sampleModel, const IReceptionAnalogModel *analogModel) :
    ReceptionResult(reception, decisions, nullptr),
    packetModel(packetModel),
    bitModel(bitModel),
    symbolModel(symbolModel),
    sampleModel(sampleModel),
    analogModel(analogModel)
{
}

LayeredReceptionResult::~LayeredReceptionResult()
{
    delete packetModel->getPacket();
    delete packetModel;
    delete bitModel;
    delete symbolModel;
    delete sampleModel;
    delete analogModel;
}

std::ostream& LayeredReceptionResult::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LayeredReceptionResult";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(packetModel, printFieldToString(packetModel, level + 1, evFlags))
               << EV_FIELD(bitModel, printFieldToString(bitModel, level + 1, evFlags))
               << EV_FIELD(symbolModel, printFieldToString(symbolModel, level + 1, evFlags))
               << EV_FIELD(sampleModel, printFieldToString(sampleModel, level + 1, evFlags))
               << EV_FIELD(analogModel, printFieldToString(analogModel, level + 1, evFlags));
    return stream;
}

const Packet *LayeredReceptionResult::getPacket() const
{
    return packetModel->getPacket();
}

} /* namespace physicallayer */
} /* namespace inet */

