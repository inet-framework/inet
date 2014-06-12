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

#include "GenericImplementation.h"

using namespace radio;

Define_Module(IsotropicRadioAntenna);
Define_Module(ConstantGainRadioAntenna);
Define_Module(DipoleRadioAntenna);
Define_Module(InterpolatingRadioAntenna);
Define_Module(ImmediateRadioSignalPropagation);
Define_Module(ConstantSpeedRadioSignalPropagation);

void ConstantGainRadioAntenna::initialize(int stage)
{
    RadioAntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        gain = FWMath::dB2fraction(par("gain"));
}

void DipoleRadioAntenna::initialize(int stage)
{
    RadioAntennaBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        length = m(par("length"));
}

double DipoleRadioAntenna::computeGain(EulerAngles direction) const
{
    double q = sin(direction.beta - M_PI_2);
    return 1.5 * q * q;
}

void RadioSignalListeningDecision::printToStream(std::ostream &stream) const
{
    stream << "listening decision, " << (isListeningPossible_ ? "possible" : "impossible");
}

void RadioSignalReceptionDecision::printToStream(std::ostream &stream) const
{
    stream << "reception decision, " << (isReceptionPossible_ ? "possible" : "impossible");
    stream << ", " << (isReceptionSuccessful_ ? "successful" : "unsuccessful");
    stream << ", indication = " << indication;
}

ImmediateRadioSignalPropagation::ImmediateRadioSignalPropagation() :
    RadioSignalPropagationBase()
{}

const IRadioSignalArrival *ImmediateRadioSignalPropagation::computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const Coord position = mobility->getCurrentPosition();
    const EulerAngles orientation = mobility->getCurrentAngularPosition();
    return new RadioSignalArrival(0.0, 0.0, transmission->getStartTime(), transmission->getEndTime(), position, position, orientation, orientation);
}

void ImmediateRadioSignalPropagation::printToStream(std::ostream &stream) const
{
    stream << "immediate radio signal propagation, theoretical propagation speed = " << propagationSpeed;
}

ConstantSpeedRadioSignalPropagation::ConstantSpeedRadioSignalPropagation() :
    RadioSignalPropagationBase(),
    mobilityApproximationCount(0)
{}

const Coord ConstantSpeedRadioSignalPropagation::computeArrivalPosition(const simtime_t time, const Coord position, IMobility *mobility) const
{
    switch (mobilityApproximationCount)
    {
        case 0:
            return mobility->getCurrentPosition();
// TODO: revive
//        case 1:
//            return mobility->getPosition(time);
//        case 2:
//        {
//            // NOTE: repeat once again to approximate the movement during propagation
//            double distance = position.distance(mobility->getPosition(time));
//            simtime_t propagationTime = distance / propagationSpeed.get();
//            return mobility->getPosition(time + propagationTime);
//        }
        default:
            throw cRuntimeError("Unknown mobility approximation count '%d'", mobilityApproximationCount);
    }
}

void ConstantSpeedRadioSignalPropagation::printToStream(std::ostream &stream) const
{
    stream << "constant speed radio signal propagation"
           << ", theoretical propagation speed = " << propagationSpeed
           << ", mobility approximation count = " << mobilityApproximationCount;
}

const IRadioSignalArrival *ConstantSpeedRadioSignalPropagation::computeArrival(const IRadioSignalTransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;
    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const Coord startPosition = transmission->getStartPosition();
    const Coord endPosition = transmission->getEndPosition();
    const Coord startArrivalPosition = computeArrivalPosition(startTime, startPosition, mobility);
    const Coord endArrivalPosition = computeArrivalPosition(endTime, endPosition, mobility);
    const simtime_t startPropagationTime = startPosition.distance(startArrivalPosition) / propagationSpeed.get();
    const simtime_t endPropagationTime = endPosition.distance(endArrivalPosition) / propagationSpeed.get();
    const simtime_t startArrivalTime = startTime + startPropagationTime;
    const simtime_t endArrivalTime = endTime + endPropagationTime;
    const EulerAngles startArrivalOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endArrivalOrientation = mobility->getCurrentAngularPosition();
    return new RadioSignalArrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
}
