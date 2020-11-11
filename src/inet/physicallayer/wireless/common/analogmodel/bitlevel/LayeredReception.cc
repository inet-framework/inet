//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/LayeredReception.h"

namespace inet {
namespace physicallayer {

LayeredReception::LayeredReception(const IReceptionAnalogModel *analogModel, const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation) :
    ReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
    analogModel(analogModel)
{
}

LayeredReception::~LayeredReception()
{
    delete analogModel;
}

std::ostream& LayeredReception::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LayeredReception";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(analogModel, printFieldToString(analogModel, level + 1, evFlags));
    return ReceptionBase::printToStream(stream, level);

}

} // namespace physicallayer
} // namespace inet

