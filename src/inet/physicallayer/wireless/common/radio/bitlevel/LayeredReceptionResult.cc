//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

