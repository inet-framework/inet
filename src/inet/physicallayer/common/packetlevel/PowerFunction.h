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

#ifndef __INET_POWERFUNCTION_H_
#define __INET_POWERFUNCTION_H_

#include "inet/common/math/Functions.h"

namespace inet {

namespace physicallayer {

class INET_API AttenuationFunction : public math::Function<double, simtime_t, Hz>
{
  protected:
    const IRadioMedium *radioMedium = nullptr;
    const double transmitterAntennaGain;
    const double receiverAntennaGain;
    const Coord transmissionPosition;
    const Coord receptionPosition;
    m distance;

  public:
    AttenuationFunction(const IRadioMedium *radioMedium, const double transmitterAntennaGain, const double receiverAntennaGain, const Coord transmissionPosition, const Coord receptionPosition) :
        radioMedium(radioMedium), transmitterAntennaGain(transmitterAntennaGain), receiverAntennaGain(receiverAntennaGain), transmissionPosition(transmissionPosition), receptionPosition(receptionPosition)
    {
        distance = m(transmissionPosition.distance(receptionPosition));
    }

    virtual double getValue(const math::Point<simtime_t, Hz>& p) const override {
        auto frequency = std::get<1>(p);
        auto propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
        auto pathLoss = radioMedium->getPathLoss()->computePathLoss(propagationSpeed, frequency, distance);
        // TODO: obstacle loss is time dependent
        auto obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(frequency, transmissionPosition, receptionPosition) : 1;
        return std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
    }

    virtual void iterateInterpolatable(const math::Interval<simtime_t, Hz>& i, const std::function<void (const math::Interval<simtime_t, Hz>& i)> f) const override {
        // TODO: this function is continuous in frequency, so what should we do?
        f(i);
    }
};

/**
 * This mathematical function provides the spectral density in the range [0, 1]
 * for any given frequency.
 */
// TODO: this could be done with a OneDimensionalInterpolatedFunction
class INET_API SpectralDensityFunction : public math::Function<double, Hz>
{
  protected:
    const Hz bandwidth;

  public:
    SpectralDensityFunction(const Hz bandwidth) : bandwidth(bandwidth) { }

    virtual math::Interval<double> getRange() const override {
        return math::Interval<double>(math::Point<double>(0), math::Point<double>(1));
    }

    virtual double getValue(const math::Point<Hz>& p) const override {
        Hz frequency = std::get<0>(p);
        return -bandwidth / 2 <= frequency && frequency <= bandwidth / 2 ? 1 : 0;
    }
};

// TODO: this could be done with a OneDimensionalBoxcarFunction and a OrthogonalCombinatorFunction
class INET_API SignalPowerFunction : public math::Function<W, simtime_t, Hz>
{
  protected:
    const W power;
    const simtime_t duration;
    const math::Function<double, Hz> *spectralDensityFunction; // shared between SignalPowerFunctions

  public:
    virtual math::Interval<W> getRange() const override {
        return math::Interval<W>(math::Point<W>(W(0)), math::Point<W>(power));
    }

    virtual W getValue(const math::Point<simtime_t, Hz>& p) const override {
        simtime_t time = std::get<0>(p);
        Hz frequency = std::get<1>(p);
        return SimTime::ZERO <= time && time <= duration ? power * spectralDensityFunction->getValue(math::Point<Hz>(frequency)) : W(0);
    }
};

/**
 * This mathematical function provides the transmission signal power for any given
 * space, time, and frequency coordinate vector.
 */
class INET_API ReceptionPowerFunction : public math::Function<W, m, m, m, simtime_t, Hz>
{
  protected:
    const mps propagationSpeed;
    const math::Point<m, m, m> startPosition;
    const math::Function<W, simtime_t, Hz> *transmissionPowerFunction;
    const math::Function<double, mps, m, Hz> *pathLossFunction; // shared between ReceptionPowerFunctions
    const math::Function<double, m, m, m, m, m, m, Hz> *obstacleLossFunction; // shared between ReceptionPowerFunctions

  public:
    ReceptionPowerFunction(mps propagationSpeed, math::Point<m, m, m> startPosition, math::Function<W, simtime_t, Hz> *transmissionPowerFunction, math::Function<double, mps, m, Hz> *pathLossFunction) :
        propagationSpeed(propagationSpeed), startPosition(startPosition), transmissionPowerFunction(transmissionPowerFunction), pathLossFunction(pathLossFunction) { }

    virtual W getValue(const math::Point<m, m, m, simtime_t, Hz>& p) const override {
        m x = std::get<0>(p);
        m y = std::get<1>(p);
        m z = std::get<2>(p);
        m startX = std::get<0>(startPosition);
        m startY = std::get<1>(startPosition);
        m startZ = std::get<2>(startPosition);
        m dx = x - startX;
        m dy = y - startY;
        m dz = z - startZ;
        simtime_t time = std::get<3>(p);
        Hz frequency = std::get<4>(p);
        m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
        simtime_t propagationTime = s(distance / propagationSpeed).get();
        W transmissionPower = transmissionPowerFunction->getValue(math::Point<simtime_t, Hz>(time - propagationTime, frequency));
        double pathLoss = pathLossFunction->getValue(math::Point<mps, m, Hz>(propagationSpeed, distance, frequency));
        double obstacleLoss = obstacleLossFunction->getValue(math::Point<m, m, m, m, m, m, Hz>(startX, startY, startZ, x, y, z, frequency));
        // TODO: add transmitter and receiver antenna gains
        return transmissionPower * pathLoss * obstacleLoss;
    }

    virtual math::Function<W, m, m, m, simtime_t, Hz> *limitDomain(const math::Interval< m, m, m, simtime_t, Hz>& i) const {
        if (std::get<0>(i.getLower()) == std::get<0>(i.getUpper()) &&
            std::get<1>(i.getLower()) == std::get<1>(i.getUpper()) &&
            std::get<2>(i.getLower()) == std::get<2>(i.getUpper()))
        {
            m x = std::get<0>(i.getLower());
            m y = std::get<1>(i.getLower());
            m z = std::get<2>(i.getLower());
            m startX = std::get<0>(startPosition);
            m startY = std::get<1>(startPosition);
            m startZ = std::get<2>(startPosition);
            m dx = x - startX;
            m dy = y - startY;
            m dz = z - startZ;
            Hz frequency = std::get<4>(i.getLower());
            m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
            double pathLoss = pathLossFunction->getValue(math::Point<mps, m, Hz>(propagationSpeed, distance, frequency));
            double obstacleLoss = obstacleLossFunction->getValue(math::Point<m, m, m, m, m, m, Hz>(startX, startY, startZ, x, y, z, frequency));
            double totalLoss = pathLoss * obstacleLoss;
            // TODO: if totalLoss is too high, simply return a ConstantFunction(0) which can be ignored in a SumFunction
            auto f = math::Function<W, simtime_t, Hz>::multiply(transmissionPowerFunction, new math::ConstantFunction<double, simtime_t, Hz>(totalLoss));
            // TODO: extend dimensions
            return nullptr;
        }
        else
            return new math::DomainLimitedFunction<W, m, m, m, simtime_t, Hz>(this, i.intersect(getDomain()));
    }

    virtual void iterateInterpolatable(const math::Interval<m, m, m, simtime_t, Hz>& i, const std::function<void (const math::Interval<m, m, m, simtime_t, Hz>& i)>) const override {
        throw cRuntimeError("TODO");
    }
};

class INET_API TransmissionMediumPowerFunction : public math::SumFunction<W, m, m, m, simtime_t, Hz>
{
  public:
    void addTransmission(const simtime_t startTime, const Hz carrierFrequency, mps propagationSpeed, const math::Point<m, m, m> startPosition, const math::Function<W, simtime_t, Hz> *signalPowerFunction, math::Function<double, mps, m, Hz> *pathLossFunction) {
        auto transmissionPowerFunction = new math::ShiftFunction<W, simtime_t, Hz>(signalPowerFunction, math::Point<simtime_t, Hz>(startTime, carrierFrequency));
        auto receptionPowerFunction = new ReceptionPowerFunction(propagationSpeed, startPosition, transmissionPowerFunction, pathLossFunction);
        fs.push_back(receptionPowerFunction);
    }
};

// TODO: power function should work the following way

//class Medium
//{
//    Function<W, m, m, m, simtime_t, Hz> powerDensity;
//};
//
//class Transmission
//{
//    Function<W, simtime_t, Hz> powerDensity;
//};
//
//void Medium::transmit(Transmission *transmission)
//{
//    medium->powerDensity->add(new PropagationAndAttenuationFunction(transmission->getPowerDensity()));
//}
//
//void Medium::receive(Transmission *transmission)
//{
//    Interval *receptionInterval; // mobility + propagation + duration + listening
//    Function *reception = new PropagationAndAttenuationFunction(transmission->getPowerDensity())->limitDomain(receptionInterval)
//    Function *total = medium->powerDensity->limitDomain(receptionInterval);
//    Function *interference = subtract(total, reception);
//    Function *snir = divide(reception, interference);
//
//    // receiver from here
//    for (Interval i : symbolIntervals(receptionInterval)) {
//        if (dbl() < snir2ser(snir->getAverage(i)))
//            ; // mark incorrect symbol
//    }
//}

} // namespace physicallayer

} // namespace inet

#endif // #ifndef __INET_POWERFUNCTION_H_

