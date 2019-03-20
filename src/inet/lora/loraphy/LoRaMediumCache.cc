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

#include "inet/lora/loraphy/LoRaMediumCache.h"

#include <algorithm>

#include "inet/lora/loraphy/LoRaLogNormalShadowing.h"
#include "inet/lora/loraphy/LoRaMedium.h"

namespace inet {

namespace lora {

Define_Module(LoRaMediumCache);

template<typename T> inline T minIgnoreNaN(T a, T b)
{
    if (std::isnan(a.get()))
        return b;
    else if (std::isnan(b.get()))
        return a;
    else if (a < b)
        return a;
    else
        return b;
}

template<typename T> inline T maxIgnoreNaN(T a, T b)
{
    if (std::isnan(a.get()))
        return b;
    else if (std::isnan(b.get()))
        return a;
    else if (a > b)
        return a;
    else
        return b;
}

inline double maxIgnoreNaN(double a, double b)
{
    if (std::isnan(a))
        return b;
    else if (std::isnan(b))
        return a;
    else if (a > b)
        return a;
    else
        return b;
}

LoRaMediumCache::LoRaMediumCache() :
    radioMedium(nullptr),
    minConstraintArea(Coord::NIL),
    maxConstraintArea(Coord::NIL),
    maxSpeed(mps(NaN)),
    maxTransmissionPower(W(NaN)),
    minInterferencePower(W(NaN)),
    minReceptionPower(W(NaN)),
    maxAntennaGain(NaN),
    minInterferenceTime(NaN),
    maxTransmissionDuration(NaN),
    maxCommunicationRange(m(NaN)),
    maxInterferenceRange(m(NaN))
{
}

void LoRaMediumCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = check_and_cast<LoRaMedium *>(getParentModule());
        WATCH(minConstraintArea);
        WATCH(maxConstraintArea);
        WATCH(maxSpeed);
        WATCH(maxTransmissionPower);
        WATCH(minInterferencePower);
        WATCH(minReceptionPower);
        WATCH(maxAntennaGain);
        WATCH(minInterferenceTime);
        WATCH(maxTransmissionDuration);
        WATCH(maxCommunicationRange);
        WATCH(maxInterferenceRange);
    }
}

std::ostream& LoRaMediumCache::printToStream(std::ostream &stream, int level) const
{
    stream << "LoRaMediumCache ";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", minConstraintArea = " << minConstraintArea
               << ", maxConstraintArea = " << maxConstraintArea
               << ", maxSpeed = " << maxSpeed
               << ", maxTransmissionPower = " << maxTransmissionPower
               << ", minInterferencePower = " << minInterferencePower
               << ", minReceptionPower = " << minReceptionPower
               << ", maxAntennaGain = " << maxAntennaGain
               << ", minInterferenceTime = " << minInterferenceTime
               << ", maxTransmissionDuration = " << maxTransmissionDuration
               << ", maxCommunicationRange = " << maxCommunicationRange
               << ", maxInterferenceRange = " << maxInterferenceRange;
    return stream;
}

void LoRaMediumCache::updateLimits()
{
    minConstraintArea = computeMinConstraintArea();
    maxConstraintArea = computeMaxConstreaintArea();
    maxSpeed = computeMaxSpeed();
    maxTransmissionPower = computeMaxTransmissionPower();
    minInterferencePower = computeMinInterferencePower();
    minReceptionPower = computeMinReceptionPower();
    maxAntennaGain = computeMaxAntennaGain();
    minInterferenceTime = computeMinInterferenceTime();
    maxTransmissionDuration = computeMaxTransmissionDuration();
    maxCommunicationRange = getMaxCommunicationRange();
    maxInterferenceRange = computeMaxInterferenceRange();
}

void LoRaMediumCache::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    updateLimits();
}

void LoRaMediumCache::removeRadio(const IRadio *radio)
{
    radios.erase(std::remove(radios.begin(), radios.end(), radio), radios.end());
    updateLimits();
}

mps LoRaMediumCache::computeMaxSpeed() const
{
    mps maxSpeed = mps(par("maxSpeed"));
    for (const auto radio : radios) {
        if (radio != nullptr)
            maxSpeed = maxIgnoreNaN(maxSpeed, mps(radio->getAntenna()->getMobility()->getMaxSpeed()));
    }
    return maxSpeed;
}

W LoRaMediumCache::computeMaxTransmissionPower() const
{
    W maxTransmissionPower = W(par("maxTransmissionPower"));
    for (const auto radio : radios) {
        if (radio != nullptr)
            maxTransmissionPower = maxIgnoreNaN(maxTransmissionPower, radio->getTransmitter()->getMaxPower());
    }
    return maxTransmissionPower;
}

W LoRaMediumCache::computeMinInterferencePower() const
{
    W minInterferencePower = mW(math::dBm2mW(par("minInterferencePower")));
    for (const auto radio : radios) {
        if (radio != nullptr)
            minInterferencePower = minIgnoreNaN(minInterferencePower, radio->getReceiver()->getMinInterferencePower());
    }
    return minInterferencePower;
}

W LoRaMediumCache::computeMinReceptionPower() const
{
    W minReceptionPower = mW(math::dBm2mW(par("minReceptionPower")));
    for (const auto radio : radios) {
        if (radio != nullptr)
            minReceptionPower = minIgnoreNaN(minReceptionPower, radio->getReceiver()->getMinReceptionPower());
    }
    return minReceptionPower;
}

double LoRaMediumCache::computeMaxAntennaGain() const
{
    double maxAntennaGain = math::dB2fraction(par("maxAntennaGain"));
    for (const auto radio : radios) {
        if (radio != nullptr)
            maxAntennaGain = maxIgnoreNaN(maxAntennaGain, radio->getAntenna()->getGain()->getMaxGain());
    }
    return maxAntennaGain;
}

m LoRaMediumCache::computeMaxRange(W maxTransmissionPower, W minReceptionPower) const
{
    // TODO: this is NaN by default
    Hz carrierFrequency = Hz(par("carrierFrequency"));
    double loss = unit(minReceptionPower / maxTransmissionPower).get() / maxAntennaGain / maxAntennaGain;
    return radioMedium->getPathLoss()->computeRange(radioMedium->getPropagation()->getPropagationSpeed(), carrierFrequency, loss);
}

m LoRaMediumCache::computeMaxInterferenceRange() const
{
    return maxIgnoreNaN(m(par("maxInterferenceRange")), computeMaxRange(maxTransmissionPower, minInterferencePower));
}

const simtime_t LoRaMediumCache::computeMinInterferenceTime() const
{
    return par("minInterferenceTime").doubleValue();
}

const simtime_t LoRaMediumCache::computeMaxTransmissionDuration() const
{
    return par("maxTransmissionDuration").doubleValue();
}

Coord LoRaMediumCache::computeMinConstraintArea() const
{
    Coord minConstraintArea = Coord::NIL;
    for (const auto radio : radios) {
        if (radio != nullptr) {
            const IMobility *mobility = radio->getAntenna()->getMobility();
            minConstraintArea = minConstraintArea.min(mobility->getConstraintAreaMin());
        }
    }
    return minConstraintArea;
}

Coord LoRaMediumCache::computeMaxConstreaintArea() const
{
    Coord maxConstraintArea = Coord::NIL;
    for (const auto radio : radios) {
        if (radio != nullptr) {
            const IMobility *mobility = radio->getAntenna()->getMobility();
            maxConstraintArea = maxConstraintArea.max(mobility->getConstraintAreaMax());
        }
    }
    return maxConstraintArea;
}

m LoRaMediumCache::getMaxInterferenceRange(const IRadio* radio) const
{
    m maxInterferenceRange = computeMaxRange(radio->getTransmitter()->getMaxPower(), minInterferencePower);
    if (!std::isnan(maxInterferenceRange.get()))
        return maxInterferenceRange;
    return radio->getTransmitter()->getMaxInterferenceRange();
}

m LoRaMediumCache::getMaxCommunicationRange(const IRadio* radio) const
{
    if (strcmp(radioMedium->par("pathLossType").stringValue(), "LoRaLogNormalShadowing") == 0) {
        LoRaLogNormalShadowing *loraLogNormalShadowing;
        loraLogNormalShadowing = check_and_cast<LoRaLogNormalShadowing *>(radioMedium->getSubmodule("pathLoss"));
        return(loraLogNormalShadowing->computeRange(maxTransmissionPower));

    }
    throw cRuntimeError("Lora propagation model is not of the type LoRaLogNormalShadowing, correct or modify the code in getMaxCommunicationRange");
    return m(NaN);
}

} // namespace physicallayer

} // namespace inet

