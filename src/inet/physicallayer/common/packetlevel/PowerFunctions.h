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

#ifndef __INET_POWERFUNCTIONS_H_
#define __INET_POWERFUNCTIONS_H_

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/math/Functions.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

/**
 * This mathematical function provides the signal attenuation for any given
 * time and frequency coordinates. This function assumes the transmitter and
 * the receiver are stationary for the duration of the transmission and the
 * reception. The function value is not time dependent but having the time
 * dimension in this definition simplifies further computations.
 */
class INET_API FrequencyDependentAttenuationFunction : public FunctionBase<double, Domain<simsec, Hz>>
{
  protected:
    const IRadioMedium *radioMedium = nullptr;
    const double transmitterAntennaGain;
    const double receiverAntennaGain;
    const Coord transmissionPosition;
    const Coord receptionPosition;
    const m distance;

  public:
    FrequencyDependentAttenuationFunction(const IRadioMedium *radioMedium, const double transmitterAntennaGain, const double receiverAntennaGain, const Coord transmissionPosition, const Coord receptionPosition) :
        radioMedium(radioMedium), transmitterAntennaGain(transmitterAntennaGain), receiverAntennaGain(receiverAntennaGain), transmissionPosition(transmissionPosition), receptionPosition(receptionPosition), distance(m(transmissionPosition.distance(receptionPosition)))
    {
    }

    virtual double getValue(const Point<simsec, Hz>& p) const override {
        Hz frequency = std::get<1>(p);
        auto propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
        auto pathLoss = radioMedium->getPathLoss()->computePathLoss(propagationSpeed, frequency, distance);
        auto obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(frequency, transmissionPosition, receptionPosition) : 1;
        double gain = transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss;
        if (gain > 1.0) {
            EV_STATICCONTEXT;
            EV_WARN << "Signal power attenuation is zero.\n";
            gain = 1.0;
        }
        return gain;
    }

    virtual bool isFinite(const Interval<simsec, Hz>& i) const override { return true; }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(FrequencyDependentAttenuation, distance = " << distance << ", transmitterAntennaGain = " << transmitterAntennaGain << ", receiverAntennaGain = " << receiverAntennaGain;
        os << "\n" << std::string(level + 2, ' ');
        os << "(" << *radioMedium->getPathLoss() << ")";
        os << "\n" << std::string(level + 2, ' ');
        os << "(" << *radioMedium->getObstacleLoss() << "))";
    }
};

/**
 * This mathematical function provides the transmission signal attenuation for any given
 * space, time, and frequency coordinates. The function value is only dependent on the
 * space and frequency coordinates. This function assumes the transmitter and is stationary
 * for the duration of the transmission. The function value is not time dependent but having
 * the time dimension in this definition simplifies further computations.
 */
class INET_API SpaceAndFrequencyDependentAttenuationFunction : public FunctionBase<double, Domain<m, m, m, simsec, Hz>>
{
  protected:
    const Ptr<const IFunction<double, Domain<Quaternion>>> transmitterAntennaGainFunction;
    const Ptr<const IFunction<double, Domain<mps, m, Hz>>> pathLossFunction;
    const Ptr<const IFunction<double, Domain<m, m, m, m, m, m, Hz>>> obstacleLossFunction;
    const Point<m, m, m> startPosition;
    const Quaternion startOrientation;
    const mps propagationSpeed;

  public:
    SpaceAndFrequencyDependentAttenuationFunction(const Ptr<const IFunction<double, Domain<Quaternion>>>& transmitterAntennaGainFunction, const Ptr<const IFunction<double, Domain<mps, m, Hz>>>& pathLossFunction, const Ptr<const IFunction<double, Domain<m, m, m, m, m, m, Hz>>>& obstacleLossFunction, const Point<m, m, m> startPosition, const Quaternion startOrientation, const mps propagationSpeed) :
        transmitterAntennaGainFunction(transmitterAntennaGainFunction), pathLossFunction(pathLossFunction), obstacleLossFunction(obstacleLossFunction), startPosition(startPosition), startOrientation(startOrientation), propagationSpeed(propagationSpeed) { }

    virtual double getValue(const Point<m, m, m, simsec, Hz>& p) const override {
        m x = std::get<0>(p);
        m y = std::get<1>(p);
        m z = std::get<2>(p);
        m startX = std::get<0>(startPosition);
        m startY = std::get<1>(startPosition);
        m startZ = std::get<2>(startPosition);
        m dx = x - startX;
        m dy = y - startY;
        m dz = z - startZ;
        Hz frequency = std::get<4>(p);
        m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
        auto direction = Quaternion::rotationFromTo(Coord::X_AXIS, Coord(dx.get(), dy.get(), dz.get()));
        auto antennaLocalDirection = startOrientation.inverse() * direction;
        double transmitterAntennaGain = distance == m(0) ? 1 : transmitterAntennaGainFunction->getValue(Point<Quaternion>(antennaLocalDirection));
        double pathLoss = pathLossFunction->getValue(Point<mps, m, Hz>(propagationSpeed, distance, frequency));
        double obstacleLoss = obstacleLossFunction != nullptr ? obstacleLossFunction->getValue(Point<m, m, m, m, m, m, Hz>(startX, startY, startZ, x, y, z, frequency)) : 1;
        double gain = transmitterAntennaGain * pathLoss * obstacleLoss;
        if (gain > 1.0) {
            EV_STATICCONTEXT;
            EV_WARN << "Signal power attenuation is zero.\n";
            gain = 1.0;
        }
        return gain;
    }

    virtual void partition(const Interval<m, m, m, simsec, Hz>& i, const std::function<void (const Interval<m, m, m, simsec, Hz>&, const IFunction<double, Domain<m, m, m, simsec, Hz>> *)> f) const override {
        const auto& lower = i.getLower();
        const auto& upper = i.getUpper();
        if ((i.getFixed() & 0b11101) == 0b11101) {
            auto lowerValue = getValue(lower);
            auto upperValue = getValue(upper);
            ASSERT(lowerValue == upperValue);
            ConstantFunction<double, Domain<m, m, m, simsec, Hz>> g(lowerValue);
            f(i, &g);
        }
        else
            FunctionBase::partition(i, f);
    }

    virtual bool isFinite(const Interval<m, m, m, simsec, Hz>& i) const override { return true; }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(SpaceAndFrequencyDependentAttenuation\n" << std::string(level + 2, ' ');
        transmitterAntennaGainFunction->printStructure(os, level + 2);
        os << "\n" << std::string(level + 2, ' ');
        pathLossFunction->printStructure(os, level + 2);
        os << "\n" << std::string(level + 2, ' ');
        obstacleLossFunction->printStructure(os, level + 2);
        os << ")";
    }
};

/**
 * This mathematical function provides the transmission signal attenuation for any given
 * space, time, and frequency coordinates. The function value is only dependent on the
 * space coordinates. This function assumes the transmitter and is stationary for the
 * duration of the transmission. The function value is not frequency and time dependent
 * but having the frequency and time dimensions in this definition simplifies further
 * computations.
 */
class INET_API SpaceDependentAttenuationFunction : public FunctionBase<double, Domain<m, m, m, simsec, Hz>>
{
  protected:
    const Ptr<const IFunction<double, Domain<Quaternion>>> transmitterAntennaGainFunction;
    const Ptr<const IFunction<double, Domain<mps, m, Hz>>> pathLossFunction;
    const Ptr<const IFunction<double, Domain<m, m, m, m, m, m, Hz>>> obstacleLossFunction;
    const Point<m, m, m> startPosition;
    const Quaternion startOrientation;
    const mps propagationSpeed;
    const Hz frequency;

  public:
    SpaceDependentAttenuationFunction(const Ptr<const IFunction<double, Domain<Quaternion>>>& transmitterAntennaGainFunction, const Ptr<const IFunction<double, Domain<mps, m, Hz>>>& pathLossFunction, const Ptr<const IFunction<double, Domain<m, m, m, m, m, m, Hz>>>& obstacleLossFunction, const Point<m, m, m> startPosition, const Quaternion startOrientation, const mps propagationSpeed, Hz frequency) :
        transmitterAntennaGainFunction(transmitterAntennaGainFunction), pathLossFunction(pathLossFunction), obstacleLossFunction(obstacleLossFunction), startPosition(startPosition), startOrientation(startOrientation), propagationSpeed(propagationSpeed), frequency(frequency) { }

    virtual double getValue(const Point<m, m, m, simsec, Hz>& p) const override {
        m x = std::get<0>(p);
        m y = std::get<1>(p);
        m z = std::get<2>(p);
        m startX = std::get<0>(startPosition);
        m startY = std::get<1>(startPosition);
        m startZ = std::get<2>(startPosition);
        m dx = x - startX;
        m dy = y - startY;
        m dz = z - startZ;
        m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
        auto direction = Quaternion::rotationFromTo(Coord::X_AXIS, Coord(dx.get(), dy.get(), dz.get()));
        auto antennaLocalDirection = startOrientation.inverse() * direction;
        double transmitterAntennaGain = distance == m(0) ? 1 : transmitterAntennaGainFunction->getValue(Point<Quaternion>(antennaLocalDirection));
        double pathLoss = pathLossFunction->getValue(Point<mps, m, Hz>(propagationSpeed, distance, frequency));
        double obstacleLoss = obstacleLossFunction != nullptr ? obstacleLossFunction->getValue(Point<m, m, m, m, m, m, Hz>(startX, startY, startZ, x, y, z, frequency)) : 1;
        double gain = transmitterAntennaGain * pathLoss * obstacleLoss;
        if (gain > 1.0) {
            EV_STATICCONTEXT;
            EV_WARN << "Signal power attenuation is zero.\n";
            gain = 1.0;
        }
        return gain;
    }

    virtual void partition(const Interval<m, m, m, simsec, Hz>& i, const std::function<void (const Interval<m, m, m, simsec, Hz>&, const IFunction<double, Domain<m, m, m, simsec, Hz>> *)> f) const override {
        const auto& lower = i.getLower();
        const auto& upper = i.getUpper();
        if ((i.getFixed() & 0b11100) == 0b11100) {
            auto lowerValue = getValue(lower);
            auto upperValue = getValue(upper);
            ASSERT(lowerValue == upperValue);
            ConstantFunction<double, Domain<m, m, m, simsec, Hz>> g(lowerValue);
            f(i, &g);
        }
        else
            FunctionBase::partition(i, f);
    }

    virtual bool isFinite(const Interval<m, m, m, simsec, Hz>& i) const override { return true; }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(SpaceDependentAttenuation\n" << std::string(level + 2, ' ');
        transmitterAntennaGainFunction->printStructure(os, level + 2);
        os << "\n" << std::string(level + 2, ' ');
        pathLossFunction->printStructure(os, level + 2);
        os << "\n" << std::string(level + 2, ' ');
        obstacleLossFunction->printStructure(os, level + 2);
        os << ")";
    }
};

/**
 * This mathematical function provides the background noise power over space, time, and frequency.
 */
class INET_API BackgroundNoisePowerFunction : public FunctionBase<WpHz, Domain<m, m, m, simsec, Hz>>
{
  protected:
    Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> powerFunction;

  public:
    BackgroundNoisePowerFunction(const IBackgroundNoise* backgroundNoise) {
        Hz minFrequency = Hz(0);
        Hz maxFrequency = GHz(10);
        Coord minPosition;
        Coord maxPosition;
        BandListening listening(nullptr, 0, getUpperBound<simtime_t>(), minPosition, maxPosition, (minFrequency + maxFrequency) / 2, maxFrequency - minFrequency);
        auto noise = backgroundNoise->computeNoise(&listening);
        if (auto dimensionalNoise = dynamic_cast<const DimensionalNoise *>(noise))
            powerFunction = dimensionalNoise->getPower();
        else
            throw cRuntimeError("TODO");
    }

    virtual WpHz getValue(const Point<m, m, m, simsec, Hz>& p) const override {
        return powerFunction->getValue(Point<simsec, Hz>(std::get<3>(p), std::get<4>(p)));
    }

    virtual void partition(const Interval<m, m, m, simsec, Hz>& i, const std::function<void (const Interval<m, m, m, simsec, Hz>&, const IFunction<WpHz, Domain<m, m, m, simsec, Hz>> *)> f) const override {
        const auto& lower = i.getLower();
        const auto& upper = i.getUpper();
        if ((i.getFixed() & 0b11100) == 0b11100) {
            Point<simsec, Hz> l1(std::get<3>(lower), std::get<4>(i.getLower()));
            Point<simsec, Hz> u1(std::get<3>(upper), std::get<4>(i.getUpper()));
            Interval<simsec, Hz> i1(l1, u1, i.getLowerClosed() & 0b11, i.getUpperClosed() & 0b11, i.getFixed() & 0b11);
            powerFunction->partition(i1, [&] (const Interval<simsec, Hz>& i2, const IFunction<WpHz, Domain<simsec, Hz>> *g) {
                Interval<m, m, m, simsec, Hz> i3(
                    Point<m, m, m, simsec, Hz>(std::get<0>(lower), std::get<1>(lower), std::get<2>(lower), std::get<0>(i2.getLower()), std::get<1>(i2.getLower())),
                    Point<m, m, m, simsec, Hz>(std::get<0>(upper), std::get<1>(upper), std::get<2>(upper), std::get<0>(i2.getUpper()), std::get<1>(i2.getUpper())),
                    0b11100 | i2.getLowerClosed(),
                    0b11100 | i2.getUpperClosed(),
                    0b11100 | i2.getFixed());
                if (auto cg = dynamic_cast<const ConstantFunction<WpHz, Domain<simsec, Hz>> *>(g)) {
                    ConstantFunction<WpHz, Domain<m, m, m, simsec, Hz>> h(cg->getConstantValue());
                    f(i3, &h);
                }
                else if (auto lg = dynamic_cast<const UnilinearFunction<WpHz, Domain<simsec, Hz>> *>(g)) {
                    UnilinearFunction<WpHz, Domain<m, m, m, simsec, Hz>> h(i3.getLower(), i3.getUpper(), lg->getValue(i2.getLower()), lg->getValue(i2.getUpper()), lg->getDimension() + 3);
                    f(i3, &h);
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
        else
            FunctionBase::partition(i, f);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(BackgroundNoisePower\n" << std::string(level + 2, ' ');
        powerFunction->printStructure(os, level + 2);
        os << ")";
    }
};

/**
 * This mathematical function provides the signal power of a transmission over
 * space, time and frequency. Signal power attenuation is not applied.
 */
class INET_API PropagatedTransmissionPowerFunction : public FunctionBase<WpHz, Domain<m, m, m, simsec, Hz>>
{
  protected:
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> transmissionPowerFunction;
    const Point<m, m, m> startPosition;
    const mps propagationSpeed;

  public:
    PropagatedTransmissionPowerFunction(const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>>& transmissionPowerFunction, const Point<m, m, m>& startPosition, mps propagationSpeed) : transmissionPowerFunction(transmissionPowerFunction), startPosition(startPosition), propagationSpeed(propagationSpeed) { }

    virtual const Point<m, m, m>& getStartPosition() const { return startPosition; }

    virtual WpHz getValue(const Point<m, m, m, simsec, Hz>& p) const override {
        m x = std::get<0>(p);
        m y = std::get<1>(p);
        m z = std::get<2>(p);
        m startX = std::get<0>(startPosition);
        m startY = std::get<1>(startPosition);
        m startZ = std::get<2>(startPosition);
        m dx = x - startX;
        m dy = y - startY;
        m dz = z - startZ;
        simsec time = std::get<3>(p);
        Hz frequency = std::get<4>(p);
        m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
        if (std::isinf(distance.get()))
            return WpHz(0);
        simsec propagationTime = simsec(distance / propagationSpeed);
        return transmissionPowerFunction->getValue(Point<simsec, Hz>(time - propagationTime, frequency));
    }

    virtual void partition(const Interval<m, m, m, simsec, Hz>& i, const std::function<void (const Interval<m, m, m, simsec, Hz>&, const IFunction<WpHz, Domain<m, m, m, simsec, Hz>> *)> f) const override {
        const auto& lower = i.getLower();
        const auto& upper = i.getUpper();
        if ((i.getFixed() & 0b11100) == 0b11100) {
            m x = std::get<0>(lower);
            m y = std::get<1>(lower);
            m z = std::get<2>(lower);
            m startX = std::get<0>(startPosition);
            m startY = std::get<1>(startPosition);
            m startZ = std::get<2>(startPosition);
            m dx = x - startX;
            m dy = y - startY;
            m dz = z - startZ;
            m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
            simsec propagationTime = simsec(distance / propagationSpeed);
            Point<simsec, Hz> l1(std::get<3>(lower) - propagationTime, std::get<4>(i.getLower()));
            Point<simsec, Hz> u1(std::get<3>(upper) - propagationTime, std::get<4>(i.getUpper()));
            Interval<simsec, Hz> i1(l1, u1, i.getLowerClosed() & 0b11, i.getUpperClosed() & 0b11, i.getFixed() & 0b11);
            transmissionPowerFunction->partition(i1, [&] (const Interval<simsec, Hz>& i2, const IFunction<WpHz, Domain<simsec, Hz>> *g) {
                Interval<m, m, m, simsec, Hz> i3(
                    Point<m, m, m, simsec, Hz>(std::get<0>(lower), std::get<1>(lower), std::get<2>(lower), std::get<0>(i2.getLower()) + propagationTime, std::get<1>(i2.getLower())),
                    Point<m, m, m, simsec, Hz>(std::get<0>(upper), std::get<1>(upper), std::get<2>(upper), std::get<0>(i2.getUpper()) + propagationTime, std::get<1>(i2.getUpper())),
                    0b11100 | i2.getLowerClosed(),
                    0b11100 | i2.getUpperClosed(),
                    0b11100 | i2.getFixed());
                if (auto cg = dynamic_cast<const ConstantFunction<WpHz, Domain<simsec, Hz>> *>(g)) {
                    ConstantFunction<WpHz, Domain<m, m, m, simsec, Hz>> h(cg->getConstantValue());
                    f(i3, &h);
                }
                else if (auto lg = dynamic_cast<const UnilinearFunction<WpHz, Domain<simsec, Hz>> *>(g)) {
                    UnilinearFunction<WpHz, Domain<m, m, m, simsec, Hz>> h(i3.getLower(), i3.getUpper(), lg->getValue(i2.getLower()), lg->getValue(i2.getUpper()), lg->getDimension() + 3);
                    f(i3, &h);
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
        else
            FunctionBase::partition(i, f);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        os << "(PropagatedTransmissionPower\n" << std::string(level + 2, ' ');
        transmissionPowerFunction->printStructure(os, level + 2);
        os << ")";
    }
};

/**
 * This mathematical function provides the path loss over space and frequency.
 */
class INET_API PathLossFunction : public FunctionBase<double, Domain<mps, m, Hz>>
{
  protected:
    const IPathLoss *pathLoss;

  public:
    PathLossFunction(const IPathLoss *pathLoss) : pathLoss(pathLoss) { }

    virtual double getValue(const Point<mps, m, Hz>& p) const override {
        mps propagationSpeed = std::get<0>(p);
        Hz frequency = std::get<2>(p);
        m distance = std::get<1>(p);
        return pathLoss->computePathLoss(propagationSpeed, frequency, distance);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override { os << "(" << *pathLoss << ")"; }
};

/**
 * This mathematical function provides the obstacle loss over frequency based on a straight path from start to end.
 */
class INET_API ObstacleLossFunction : public FunctionBase<double, Domain<m, m, m, m, m, m, Hz>>
{
  protected:
    const IObstacleLoss *obstacleLoss;

  public:
    ObstacleLossFunction(const IObstacleLoss *obstacleLoss) : obstacleLoss(obstacleLoss) { }

    virtual double getValue(const Point<m, m, m, m, m, m, Hz>& p) const override {
        Coord transmissionPosition(std::get<0>(p).get(), std::get<1>(p).get(), std::get<2>(p).get());
        Coord receptionPosition(std::get<3>(p).get(), std::get<4>(p).get(), std::get<5>(p).get());
        Hz frequency = std::get<6>(p);
        return obstacleLoss->computeObstacleLoss(frequency, transmissionPosition, receptionPosition);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override { os << "(" << *obstacleLoss << ")"; }
};

/**
 * This mathematical function provides the antenna gain over orientation.
 */
class INET_API AntennaGainFunction : public IFunction<double, Domain<Quaternion>>
{
  protected:
    const IAntennaGain *antennaGain;

  public:
    AntennaGainFunction(const IAntennaGain *antennaGain) : antennaGain(antennaGain) { }

    virtual Interval<double> getRange() const override { return getRange(getDomain()); }
    virtual Interval<double> getRange(const Interval<Quaternion>& i) const override { throw cRuntimeError("TODO"); }
    virtual Interval<Quaternion> getDomain() const override { throw cRuntimeError("TODO"); }

    virtual double getValue(const Point<Quaternion>& p) const override {
        return antennaGain->computeGain(std::get<0>(p));
    }

    virtual void partition(const Interval<Quaternion>& i, const std::function<void (const Interval<Quaternion>&, const IFunction<double, Domain<Quaternion>> *)> f) const override { throw cRuntimeError("TODO"); }

    virtual bool isFinite() const override { throw cRuntimeError("TODO"); }
    virtual bool isFinite(const Interval<Quaternion>& i) const override { throw cRuntimeError("TODO"); }

    virtual double getMin() const override { throw cRuntimeError("TODO"); }
    virtual double getMin(const Interval<Quaternion>& i) const override { throw cRuntimeError("TODO"); }

    virtual double getMax() const override { throw cRuntimeError("TODO"); }
    virtual double getMax(const Interval<Quaternion>& i) const override { throw cRuntimeError("TODO"); }

    virtual double getMean() const override { throw cRuntimeError("TODO"); }
    virtual double getMean(const Interval<Quaternion>& i) const override { throw cRuntimeError("TODO"); }

    virtual double getIntegral() const override { throw cRuntimeError("TODO"); }
    virtual double getIntegral(const Interval<Quaternion>& i) const override { throw cRuntimeError("TODO"); }

    virtual const Ptr<const IFunction<double, Domain<Quaternion>>> add(const Ptr<const IFunction<double, Domain<Quaternion>>>& o) const override { throw cRuntimeError("TODO"); }
    virtual const Ptr<const IFunction<double, Domain<Quaternion>>> subtract(const Ptr<const IFunction<double, Domain<Quaternion>>>& o) const override { throw cRuntimeError("TODO"); }
    virtual const Ptr<const IFunction<double, Domain<Quaternion>>> multiply(const Ptr<const IFunction<double, Domain<Quaternion>>>& o) const override { throw cRuntimeError("TODO"); }
    virtual const Ptr<const IFunction<double, Domain<Quaternion>>> divide(const Ptr<const IFunction<double, Domain<Quaternion>>>& o) const override { throw cRuntimeError("TODO"); }

    virtual void print(std::ostream& os, int level = 0) const override { os << "TODO"; }
    virtual void print(std::ostream& os, const Interval<Quaternion>& i, int level = 0) const override { os << "TODO"; }
    virtual void printPartitioning(std::ostream& os, const Interval<Quaternion>& i, int level = 0) const override { os << "TODO"; }
    virtual void printPartition(std::ostream& os, const Interval<Quaternion>& i, int level = 0) const override { os << "TODO"; }
    virtual void printStructure(std::ostream& os, int level = 0) const override { os << "(AntennaGain, minGain = " << antennaGain->getMinGain() << ", maxGain = " << antennaGain->getMaxGain() << ")"; }
};

} // namespace physicallayer

} // namespace inet

#endif // #ifndef __INET_POWERFUNCTIONS_H_

