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

#include "inet/lora/loraphy/LoraMode.h"

#include <math.h>

namespace inet {
namespace lora {

LoraMode::LoraMode(const char *name,
                    Hz bandwith,
                    int spreadingFactor,
                    int cwMin,
                    int cwMax,
                    b pading,
                    double slot,
                    double sif,
                    double difs,
                    double cca,
                    double rxStartDelay,
                    double rxTxTurn,
                    double codeRate):
                name(name),
                bandwith(bandwith),
                spreadingFactor(spreadingFactor),
                cwMin(cwMin),
                cwMax(cwMax),
                pading(pading),
                slot(slot),
                sif(sif),
                difs(difs),
                cca(cca),
                rxStartDelay(rxStartDelay),
                rxTxTurn(rxTxTurn),
                codeRate(codeRate)
{
    // TODO Auto-generated constructor stub

    bps bitRate;

    if (spreadingFactor < 7 || spreadingFactor > 12)
        throw cRuntimeError("Invalid Spreading factor");

    if (codeRate < 1 || codeRate > 4)
        throw cRuntimeError("Invalid code rate");


    double rb = (double)spreadingFactor * ((4/(4+codeRate))/(pow(2.0,(double)spreadingFactor)/bandwith.get())) * 1000;
    bitRate = bps(floor(rb));

    if (bandwith == Hz(250000)) {
        if (bitRate.get() > 3000)
            maxLeng = 230;
        else if (bitRate.get() > 1000)
            maxLeng = 123;
        else
            maxLeng = 59;

    }
    else if (bandwith == Hz(125000)) {
        if (bitRate.get() > 5000)
            maxLeng = 250;
        else if (bitRate.get() > 3000)
            maxLeng = 133;
        else if (bitRate.get() > 1000)
            maxLeng = 61;
        else
            maxLeng = 19;
    }
    else if (bandwith == Hz(500000)) {
        if (bitRate.get() > 3000)
            maxLeng = 230;
        else if (bitRate.get() > 1000)
            maxLeng = 117;
        else
            maxLeng = 41;
    }
    else
        throw cRuntimeError("Invalid bandwith");

    modulation = new LoRaModulation(spreadingFactor, bandwith, bitRate, header, codeRate);
}

const simtime_t LoraMode::getPreambleLength() const {
    simtime_t symbTime = modulation->getSymbolTyme();
    return symbTime * (preambleLength + 4.25);
}

const simtime_t LoraMode::getHeaderLength(b dataLength) const {

    if (header == 1)
        return 0;
    double payloadComputation1 = ceil(((double)dataLength.get() - (4.0*(double)spreadingFactor) + 28.0 + 16.0 - (20.0 * (double)1))/(4.0*((double)spreadingFactor-2.0)));
    double payloadComputation2 = ceil(((double)dataLength.get() - (4.0*(double)spreadingFactor) + 28.0 + 16.0 - (20.0 * (double)0))/(4.0*((double)spreadingFactor-2.0)));

    return (payloadComputation2 - payloadComputation1) * modulation->getSymbolTyme() ;
}

const simtime_t LoraMode::getPayloadLength(b dataLength) const {

    double payloadComputation1 = ceil((((double)dataLength.get()) - (4.0*(double)spreadingFactor) + 28.0 + 16.0 - (20.0 * (double)1))/(4.0*((double)spreadingFactor-2.0)));

    return modulation->getSymbolTyme() * payloadComputation1;
}

const simtime_t LoraMode::getDuration(b dataLength) const {
    double payloadComputation = ceil((((double)dataLength.get()) - (4.0*(double)spreadingFactor) + 28.0 + 16.0 - (20.0 * (double)header))/(4.0*((double)spreadingFactor-2.0)));
    payloadComputation *= (codeRate+4.0);
    if (payloadComputation < 0)
        payloadComputation = 0;

    double payloadSimbol = 8+payloadComputation;

    return (modulation->getSymbolTyme() * payloadSimbol) + getPreambleLength();
}

void LoraMode::setHeaderEnable(const bool &p)
{
    header = 1;
    if (p)
        header = 0;
    modulation->setHeaderEnable(p);
}


LoraMode::~LoraMode() {
    // TODO Auto-generated destructor stub
    delete modulation;
}


const LoraMode LoraCompliantModes::EULoraD0("EuD0",Hz(125000), 12, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 2);
const LoraMode LoraCompliantModes::EULoraD1("EuD1",Hz(125000), 11, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 2);
const LoraMode LoraCompliantModes::EULoraD2("EuD2",Hz(125000), 10, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::EULoraD3("EuD3",Hz(125000), 9, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::EULoraD4("EuD4",Hz(125000), 8, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::EULoraD5("EuD5",Hz(125000), 7, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::EULoraD6("EuD6",Hz(250000), 7, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);

const LoraMode LoraCompliantModes::USALoraD0("UsaD0", Hz(125000), 10, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 2);
const LoraMode LoraCompliantModes::USALoraD1("UsaD1", Hz(125000), 9, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 2);
const LoraMode LoraCompliantModes::USALoraD2("UsaD2", Hz(125000), 8, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::USALoraD3("UsaD3", Hz(125000), 7, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::USALoraD4("UsaD4", Hz(500000), 8, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::USALoraD8("UsaD8", Hz(500000), 12, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 2);
const LoraMode LoraCompliantModes::USALoraD9("UsaD9", Hz(500000), 11, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 2);
const LoraMode LoraCompliantModes::USALoraD10("UsaD10", Hz(500000), 10, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::USALoraD11("UsaD11", Hz(500000), 9, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::USALoraD12("UsaD12", Hz(500000), 8, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);
const LoraMode LoraCompliantModes::USALoraD13("UsaD13", Hz(500000), 7, 0, 0, b(0), 0, 0, 0, 0, 0, 0, 1);

//const std::vector<LoraMode *> LoraCompliantModes::LoraModeEu = {&LoraCompliantModes::EULoraD0, &LoraCompliantModes::EULoraD1, &LoraCompliantModes::EULoraD2,&LoraCompliantModes::EULoraD3,&LoraCompliantModes::EULoraD4,&LoraCompliantModes::EULoraD5,&LoraCompliantModes::EULoraD6};
//const std::vector<LoraMode *> LoraCompliantModes::LoraModeUsa = {&USALoraD0, &USALoraD1, &USALoraD2,&USALoraD3,&USALoraD4,&USALoraD8,&USALoraD9,&USALoraD10, &USALoraD11, &USALoraD12};

const int LoraCompliantModes::LoraModeEuTotal = 7;
const int LoraCompliantModes::LoraModeUsaTotal =11;
const LoraMode * LoraCompliantModes::LoraModeEu[] = {&LoraCompliantModes::EULoraD0, &LoraCompliantModes::EULoraD1, &LoraCompliantModes::EULoraD2,&LoraCompliantModes::EULoraD3,&LoraCompliantModes::EULoraD4,&LoraCompliantModes::EULoraD5,&LoraCompliantModes::EULoraD6};;
const LoraMode * LoraCompliantModes::LoraModeUsa[] = {&USALoraD0, &USALoraD1, &USALoraD2,&USALoraD3,&USALoraD4,&USALoraD8,&USALoraD9,&USALoraD10, &USALoraD11, &USALoraD12};;

} /* namespace physicallayer */
} /* namespace inet */
