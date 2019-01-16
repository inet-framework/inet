//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/physicallayer/common/bitlevel/LayeredReceptionResult.h"

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

std::ostream& LayeredReceptionResult::printToStream(std::ostream& stream, int level) const
{
    stream << "LayeredReceptionResult";
    if (level <= PRINT_LEVEL_TRACE)
       stream << ", packetModel = " << printObjectToString(packetModel, level + 1)
              << ", bitModel = " << printObjectToString(bitModel, level + 1)
              << ", symbolModel = " << printObjectToString(symbolModel, level + 1)
              << ", sampleModel = " << printObjectToString(sampleModel, level + 1)
              << ", analogModel = " << printObjectToString(analogModel, level + 1);
    return stream;
}

const Packet *LayeredReceptionResult::getPacket() const
{
    return packetModel->getPacket();
}

} /* namespace physicallayer */
} /* namespace inet */

