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

#include "ScalarReceiver.h"
#include "ScalarReception.h"
#include "ScalarNoise.h"
#include "BandListening.h"

namespace inet {

namespace physicallayer {
Define_Module(ScalarReceiver);

void ScalarReceiver::printToStream(std::ostream& stream) const
{
    stream << "scalar receiver, "
           << "energyDetection = " << energyDetection << ", "
           << "sensitivity = " << sensitivity << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

const INoise *ScalarReceiver::computeNoise(const IListening *listening, const std::vector<const IReception *> *receptions, const INoise *backgroundNoise) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    for (std::vector<const IReception *>::const_iterator it = receptions->begin(); it != receptions->end(); it++) {
        const ScalarReception *reception = check_and_cast<const ScalarReception *>(*it);
        if (carrierFrequency == reception->getCarrierFrequency() && bandwidth == reception->getBandwidth()) {
            W power = reception->getPower();
            simtime_t startTime = reception->getStartTime();
            simtime_t endTime = reception->getEndTime();
            if (startTime < noiseStartTime)
                noiseStartTime = startTime;
            if (endTime > noiseEndTime)
                noiseEndTime = endTime;
            std::map<simtime_t, W>::iterator itStartTime = powerChanges->find(startTime);
            if (itStartTime != powerChanges->end())
                itStartTime->second += power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
            std::map<simtime_t, W>::iterator itEndTime = powerChanges->find(endTime);
            if (itEndTime != powerChanges->end())
                itEndTime->second -= power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
        }
        else if (areOverlappingBands(carrierFrequency, bandwidth, reception->getCarrierFrequency(), reception->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    if (backgroundNoise) {
        const ScalarNoise *scalarBackgroundNoise = check_and_cast<const ScalarNoise *>(backgroundNoise);
        if (carrierFrequency == scalarBackgroundNoise->getCarrierFrequency() && bandwidth == scalarBackgroundNoise->getBandwidth()) {
            const std::map<simtime_t, W> *backgroundNoisePowerChanges = check_and_cast<const ScalarNoise *>(backgroundNoise)->getPowerChanges();
            for (std::map<simtime_t, W>::const_iterator it = backgroundNoisePowerChanges->begin(); it != backgroundNoisePowerChanges->end(); it++) {
                std::map<simtime_t, W>::iterator jt = powerChanges->find(it->first);
                if (jt != powerChanges->end())
                    jt->second += it->second;
                else
                    powerChanges->insert(std::pair<simtime_t, W>(it->first, it->second));
            }
        }
        else if (areOverlappingBands(carrierFrequency, bandwidth, scalarBackgroundNoise->getCarrierFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    return new ScalarNoise(noiseStartTime, noiseEndTime, carrierFrequency, bandwidth, powerChanges);
}

double ScalarReceiver::computeMinSNIR(const IReception *reception, const INoise *noise) const
{
    const ScalarNoise *scalarNoise = check_and_cast<const ScalarNoise *>(noise);
    const ScalarReception *scalarReception = check_and_cast<const ScalarReception *>(reception);
    return unit(scalarReception->getPower() / scalarNoise->computeMaxPower(reception->getStartTime(), reception->getEndTime())).get();
}

} // namespace physicallayer
} // namespace inet

