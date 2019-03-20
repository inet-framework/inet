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

#include "inet/lora/loraphy/LoRaHataOkumura.h"

namespace inet {

namespace lora {

Define_Module(LoRaHataOkumura);

LoRaHataOkumura::LoRaHataOkumura()
{
}

void LoRaHataOkumura::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        K1 = par("K1");
        K2 = par("K2");
    }
}

std::ostream& LoRaHataOkumura::printToStream(std::ostream& stream, int level) const
{
    stream << "LoRaHataOkumura";
    return stream;
}

double LoRaHataOkumura::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    // build based on documentation from Actility
    double PL_db = K1 + K2 * log10(distance.get()/1000);
    return math::dB2fraction(-PL_db);
}

}

}
