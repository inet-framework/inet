//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/medium/MediumLimitCache.h"

#include <algorithm>

namespace inet {
namespace physicallayer {

Define_Module(MediumLimitCache);

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

MediumLimitCache::MediumLimitCache() :
    radioMedium(nullptr),
    minConstraintArea(Coord::NIL),
    maxConstraintArea(Coord::NIL),
    maxSpeed(mps(NaN)),
    maxTransmissionPower(W(NaN)),
    minInterferencePower(W(NaN)),
    minReceptionPower(W(NaN)),
    maxAntennaGain(NaN),
    minInterferenceTime(-1),
    maxTransmissionDuration(-1),
    maxCommunicationRange(m(NaN)),
    maxInterferenceRange(m(NaN))
{
}

void MediumLimitCache::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        radioMedium = check_and_cast<IRadioMedium *>(getParentModule());
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

std::ostream& MediumLimitCache::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "RadioMediumLimits";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(minConstraintArea)
               << EV_FIELD(maxConstraintArea)
               << EV_FIELD(maxSpeed)
               << EV_FIELD(maxTransmissionPower)
               << EV_FIELD(minInterferencePower)
               << EV_FIELD(minReceptionPower)
               << EV_FIELD(maxAntennaGain)
               << EV_FIELD(minInterferenceTime)
               << EV_FIELD(maxTransmissionDuration)
               << EV_FIELD(maxCommunicationRange)
               << EV_FIELD(maxInterferenceRange);
    return stream;
}

void MediumLimitCache::updateLimits()
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
    maxCommunicationRange = computeMaxCommunicationRange();
    maxInterferenceRange = computeMaxInterferenceRange();
}

void MediumLimitCache::addRadio(const IRadio *radio)
{
    radios.push_back(radio);
    updateLimits();
}

void MediumLimitCache::removeRadio(const IRadio *radio)
{
    radios.erase(std::remove(radios.begin(), radios.end(), radio), radios.end());
    updateLimits();
}

mps MediumLimitCache::computeMaxSpeed() const
{
    mps maxSpeed = mps(par("maxSpeed"));
    for (const auto radio : radios) {
        if (radio != nullptr)
            maxSpeed = maxIgnoreNaN(maxSpeed, mps(radio->getAntenna()->getMobility()->getMaxSpeed()));
    }
    return maxSpeed;
}

W MediumLimitCache::computeMaxTransmissionPower() const
{
    W maxTransmissionPower = W(par("maxTransmissionPower"));
    for (const auto radio : radios) {
        if (radio != nullptr)
            maxTransmissionPower = maxIgnoreNaN(maxTransmissionPower, radio->getTransmitter()->getMaxPower());
    }
    return maxTransmissionPower;
}

W MediumLimitCache::computeMinInterferencePower() const
{
    W minInterferencePower = mW(math::dBmW2mW(par("minInterferencePower")));
    for (const auto radio : radios) {
        if (radio != nullptr && radio->getReceiver() != nullptr)
            minInterferencePower = minIgnoreNaN(minInterferencePower, radio->getReceiver()->getMinInterferencePower());
    }
    return minInterferencePower;
}

W MediumLimitCache::computeMinReceptionPower() const
{
    W minReceptionPower = mW(math::dBmW2mW(par("minReceptionPower")));
    for (const auto radio : radios) {
        if (radio != nullptr && radio->getReceiver() != nullptr)
            minReceptionPower = minIgnoreNaN(minReceptionPower, radio->getReceiver()->getMinReceptionPower());
    }
    return minReceptionPower;
}

double MediumLimitCache::computeMaxAntennaGain() const
{
    double maxAntennaGain = math::dB2fraction(par("maxAntennaGain"));
    for (const auto radio : radios) {
        if (radio != nullptr)
            maxAntennaGain = maxIgnoreNaN(maxAntennaGain, radio->getAntenna()->getGain()->getMaxGain());
    }
    return maxAntennaGain;
}

m MediumLimitCache::computeMaxRange(W maxTransmissionPower, W minReceptionPower) const
{
    // TODO this is NaN by default
    Hz centerFrequency = Hz(par("centerFrequency"));
    double loss = unit(minReceptionPower / maxTransmissionPower).get() / maxAntennaGain / maxAntennaGain;
    return radioMedium->getPathLoss()->computeRange(radioMedium->getPropagation()->getPropagationSpeed(), centerFrequency, loss);
}

m MediumLimitCache::computeMaxCommunicationRange() const
{
    return maxIgnoreNaN(m(par("maxCommunicationRange")), computeMaxRange(maxTransmissionPower, minReceptionPower));
}

m MediumLimitCache::computeMaxInterferenceRange() const
{
    return maxIgnoreNaN(m(par("maxInterferenceRange")), computeMaxRange(maxTransmissionPower, minInterferencePower));
}

const simtime_t MediumLimitCache::computeMinInterferenceTime() const
{
    return simtime_t(par("minInterferenceTime"));
}

const simtime_t MediumLimitCache::computeMaxTransmissionDuration() const
{
    return simtime_t(par("maxTransmissionDuration"));
}

Coord MediumLimitCache::computeMinConstraintArea() const
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

Coord MediumLimitCache::computeMaxConstreaintArea() const
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

m MediumLimitCache::getMaxInterferenceRange(const IRadio *radio) const
{
    m maxInterferenceRange = computeMaxRange(radio->getTransmitter()->getMaxPower(), minInterferencePower);
    if (!std::isnan(maxInterferenceRange.get()))
        return maxInterferenceRange;
    return radio->getTransmitter()->getMaxInterferenceRange();
}

m MediumLimitCache::getMaxCommunicationRange(const IRadio *radio) const
{
    m maxCommunicationRange = computeMaxRange(radio->getTransmitter()->getMaxPower(), minReceptionPower);
    if (!std::isnan(maxCommunicationRange.get()))
        return maxCommunicationRange;
    return radio->getTransmitter()->getMaxCommunicationRange();
}

} // namespace physicallayer
} // namespace inet

