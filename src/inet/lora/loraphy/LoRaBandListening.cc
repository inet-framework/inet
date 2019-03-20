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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "../loraphy/LoRaBandListening.h"

namespace inet {

namespace lora {


LoRaBandListening::LoRaBandListening(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition, Hz LoRaCF, int LoRaSF, Hz LoRaBW) :
    ListeningBase(radio, startTime, endTime, startPosition, endPosition),
    LoRaCF(LoRaCF),
    LoRaSF(LoRaSF),
    LoRaBW(LoRaBW)
{
}

std::ostream& LoRaBandListening::printToStream(std::ostream& stream, int level) const
{
    stream << "LoRaBandListening";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", LoRaCF = " << LoRaCF
               << ", LoRaSF = " << LoRaSF
               << ", LoRaBW = " << LoRaBW;
    return ListeningBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet
