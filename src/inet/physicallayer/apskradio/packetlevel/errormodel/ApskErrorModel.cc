//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/apskradio/packetlevel/errormodel/ApskErrorModel.h"
#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskErrorModel);

std::ostream& ApskErrorModel::printToStream(std::ostream& stream, int level) const
{
    return stream << "ApskErrorModel";
}

double ApskErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    double bitErrorRate = computeBitErrorRate(snir, part);
    if (bitErrorRate == 0.0)
        return 0.0;
    else if (bitErrorRate == 1.0)
        return 1.0;
    else {
        const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(snir->getReception()->getTransmission());
        double headerSuccessRate = pow(1.0 - bitErrorRate, b(flatTransmission->getHeaderLength()).get());
        double dataSuccessRate = pow(1.0 - bitErrorRate, b(flatTransmission->getDataLength()).get());
        switch (part) {
            case IRadioSignal::SIGNAL_PART_WHOLE:
                return 1.0 - headerSuccessRate * dataSuccessRate;
            case IRadioSignal::SIGNAL_PART_PREAMBLE:
                return 0;
            case IRadioSignal::SIGNAL_PART_HEADER:
                return 1.0 - headerSuccessRate;
            case IRadioSignal::SIGNAL_PART_DATA:
                return 1.0 - dataSuccessRate;
            default:
                throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
        }
    }
}

double ApskErrorModel::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(flatTransmission->getModulation());
    return modulation->calculateBER(getScalarSnir(snir), flatTransmission->getBandwidth(), flatTransmission->getBitrate());
}

double ApskErrorModel::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(snir->getReception()->getTransmission());
    const ApskModulationBase *modulation = check_and_cast<const ApskModulationBase *>(flatTransmission->getModulation());
    return modulation->calculateSER(getScalarSnir(snir), flatTransmission->getBandwidth(), flatTransmission->getBitrate());
}

} // namespace physicallayer

} // namespace inet

