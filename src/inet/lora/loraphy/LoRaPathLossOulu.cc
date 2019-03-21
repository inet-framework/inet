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

#include "inet/lora/loraphy/LoRaPathLossOulu.h"

namespace inet {

namespace lora {

Define_Module(LoRaPathLossOulu);

LoRaPathLossOulu::LoRaPathLossOulu()
{
}

void LoRaPathLossOulu::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        d0 = m(par("d0"));
        n = par("n");
        B = par("B");
        sigma = par("sigma");
        antennaGain = par("antennaGain");
    }
}

double LoRaPathLossOulu::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    //EPL = B + 10nlog10( d / d0 )
    //double PL_d0_db = 127.41;
    //double PL_db = PL_d0_db + 10 * gamma * log10(unit(distance / d0).get()) + normal(0.0, sigma);
    double PL_db = B + 10 * n * log10(unit(distance/d0).get()) - antennaGain + normal(0.0, sigma);
    return math::dB2fraction(-PL_db);
}

}

}
