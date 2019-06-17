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

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/math/Functions.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

using namespace inet::math;

class INET_API AttenuationFunction : public FunctionBase<double, simtime_t, Hz>
{
  protected:
    const IRadioMedium *radioMedium = nullptr;
    const double transmitterAntennaGain;
    const double receiverAntennaGain;
    const Coord transmissionPosition;
    const Coord receptionPosition;
    m distance;

  protected:
    virtual double getAttenuation(Hz frequency) const {
        auto propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
        auto pathLoss = radioMedium->getPathLoss()->computePathLoss(propagationSpeed, frequency, distance);
        auto obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(frequency, transmissionPosition, receptionPosition) : 1;
        return std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
    }

  public:
    AttenuationFunction(const IRadioMedium *radioMedium, const double transmitterAntennaGain, const double receiverAntennaGain, const Coord transmissionPosition, const Coord receptionPosition) :
        radioMedium(radioMedium), transmitterAntennaGain(transmitterAntennaGain), receiverAntennaGain(receiverAntennaGain), transmissionPosition(transmissionPosition), receptionPosition(receptionPosition)
    {
        distance = m(transmissionPosition.distance(receptionPosition));
    }

    virtual double getValue(const Point<simtime_t, Hz>& p) const override {
        return getAttenuation(std::get<1>(p));
    }

    virtual void partition(const Interval<simtime_t, Hz>& i, const std::function<void (const Interval<simtime_t, Hz>&, const IFunction<double, simtime_t, Hz> *)> f) const override {
//        Hz frequency = MHz(100) * round(unit(std::get<1>(i.getLower()) / MHz(100)).get());
//        ASSERT(frequency == MHz(100) * round(unit(std::get<1>(i.getUpper()) / MHz(100)).get()));
        Hz frequency = (std::get<1>(i.getLower()) + std::get<1>(i.getUpper())) / 2;
        ConstantFunction<double, simtime_t, Hz> g(getAttenuation(frequency));
        f(i, &g);
    }
};

/**
 * This mathematical function provides the transmission signal power for any given
 * space, time, and frequency coordinate vector.
 */
class INET_API ReceptionPowerFunction : public FunctionBase<W, m, m, m, simtime_t, Hz>
{
  protected:
    const mps propagationSpeed;
    const Point<m, m, m> startPosition;
    const Quaternion startOrientation;
    const Ptr<const IFunction<W, simtime_t, Hz>> transmissionPowerFunction;
    const Ptr<const IFunction<double, Quaternion>> transmitterAntennaGainFunction;
    const Ptr<const IFunction<double, mps, m, Hz>> pathLossFunction;
    const Ptr<const IFunction<double, m, m, m, m, m, m, Hz>> obstacleLossFunction;

  protected:
    double getAttenuation(const Point<m, m, m, simtime_t, Hz>& p) const {
        m x = std::get<0>(p);
        m y = std::get<1>(p);
        m z = std::get<2>(p);
        m startX = std::get<0>(startPosition);
        m startY = std::get<1>(startPosition);
        m startZ = std::get<2>(startPosition);
        m dx = x - startX;
        m dy = y - startY;
        m dz = z - startZ;
        Hz frequency = MHz(100) * round(unit(std::get<4>(p) / MHz(100)).get());;
        m distance = m(sqrt(dx * dx + dy * dy + dz * dz));
        auto direction = Quaternion::rotationFromTo(Coord::X_AXIS, Coord(dx.get(), dy.get(), dz.get()));
        auto antennaLocalDirection = startOrientation.inverse() * direction;
        double transmitterAntennaGain = distance == m(0) ? 1 : transmitterAntennaGainFunction->getValue(Point<Quaternion>(antennaLocalDirection));
        double pathLoss = pathLossFunction->getValue(Point<mps, m, Hz>(propagationSpeed, distance, frequency));
        double obstacleLoss = obstacleLossFunction != nullptr ? obstacleLossFunction->getValue(Point<m, m, m, m, m, m, Hz>(startX, startY, startZ, x, y, z, frequency)) : 1;
        return std::min(1.0, transmitterAntennaGain * pathLoss * obstacleLoss);
    }

  public:
    ReceptionPowerFunction(const mps propagationSpeed, const Point<m, m, m> startPosition, const Quaternion startOrientation, const Ptr<const IFunction<W, simtime_t, Hz>>& transmissionPowerFunction, const Ptr<const IFunction<double, Quaternion>>& transmitterAntennaGainFunction, const Ptr<const IFunction<double, mps, m, Hz>>& pathLossFunction, const Ptr<const IFunction<double, m, m, m, m, m, m, Hz>>& obstacleLossFunction) :
        propagationSpeed(propagationSpeed), startPosition(startPosition), startOrientation(startOrientation), transmissionPowerFunction(transmissionPowerFunction), transmitterAntennaGainFunction(transmitterAntennaGainFunction), pathLossFunction(pathLossFunction), obstacleLossFunction(obstacleLossFunction) { }

    virtual const Point<m, m, m>& getStartPosition() const { return startPosition; }

    virtual W getValue(const Point<m, m, m, simtime_t, Hz>& p) const override {
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
        if (std::isinf(distance.get()))
            return W(0);
        simtime_t propagationTime = s(distance / propagationSpeed).get();
        W transmissionPower = transmissionPowerFunction->getValue(Point<simtime_t, Hz>(time - propagationTime, frequency));
        return transmissionPower * getAttenuation(p);
    }

    virtual void partition(const Interval<m, m, m, simtime_t, Hz>& i, const std::function<void (const Interval<m, m, m, simtime_t, Hz>&, const IFunction<W, m, m, m, simtime_t, Hz> *)> f) const override {
        const auto& lower = i.getLower();
        const auto& upper = i.getUpper();
        if (std::get<0>(lower) == std::get<0>(upper) && std::get<1>(lower) == std::get<1>(upper) && std::get<2>(lower) == std::get<2>(upper)) {
            // TODO: parameter for MHz(100) quantization
            Hz frequency = MHz(100) * round(unit(std::get<4>(lower) / MHz(100)).get());
            ASSERT(frequency == MHz(100) * round(unit(std::get<4>(upper) / MHz(100)).get()));
            double attenuation = getAttenuation(Point<m, m, m, simtime_t, Hz>(std::get<0>(lower), std::get<1>(lower), std::get<2>(lower), 0, frequency));
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
            simtime_t propagationTime = s(distance / propagationSpeed).get();
            Interval<simtime_t, Hz> i1(Point<simtime_t, Hz>(std::get<3>(lower) - propagationTime, std::get<4>(lower)), Point<simtime_t, Hz>(std::get<3>(upper) - propagationTime, std::get<4>(upper)));
            transmissionPowerFunction->partition(i1, [&] (const Interval<simtime_t, Hz>& i2, const IFunction<W, simtime_t, Hz> *g) {
                Interval<m, m, m, simtime_t, Hz> i3(
                    Point<m, m, m, simtime_t, Hz>(std::get<0>(lower), std::get<1>(lower), std::get<2>(lower), std::get<0>(i2.getLower()) + propagationTime, std::get<1>(i2.getLower())),
                    Point<m, m, m, simtime_t, Hz>(std::get<0>(upper), std::get<1>(upper), std::get<2>(upper), std::get<0>(i2.getUpper()) + propagationTime, std::get<1>(i2.getUpper())));
                if (auto cg = dynamic_cast<const ConstantFunction<W, simtime_t, Hz> *>(g)) {
                    ConstantFunction<W, m, m, m, simtime_t, Hz> h(cg->getConstantValue() * attenuation);
                    f(i3, &h);
                }
                else if (auto lg = dynamic_cast<const LinearInterpolatedFunction<W, simtime_t, Hz> *>(g)) {
                    LinearInterpolatedFunction<W, m, m, m, simtime_t, Hz> h(i3.getLower(), i3.getUpper(), lg->getValue(i2.getLower()) * attenuation, lg->getValue(i2.getUpper()) * attenuation, lg->getDimension() + 3);
                    f(i3, &h);
                }
                else
                    throw cRuntimeError("TODO");
            });
        }
        else
            throw cRuntimeError("TODO");
    }
};

class INET_API PathLossFunction : public FunctionBase<double, mps, m, Hz>
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

    virtual void partition(const Interval<mps, m, Hz>& i, const std::function<void (const Interval<mps, m, Hz>&, const IFunction<double, mps, m, Hz> *)> f) const override {
        throw cRuntimeError("TODO");
    }
};

class INET_API ObstacleLossFunction : public FunctionBase<double, m, m, m, m, m, m, Hz>
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

    virtual void partition(const Interval<m, m, m, m, m, m, Hz>& i, const std::function<void (const Interval<m, m, m, m, m, m, Hz>&, const IFunction<double, m, m, m, m, m, m, Hz> *)> f) const override {
        throw cRuntimeError("TODO");
    }
};

class INET_API AntennaGainFunction : public IFunction<double, Quaternion>
{
  protected:
    const IAntennaGain *antennaGain;

  public:
    AntennaGainFunction(const IAntennaGain *antennaGain) : antennaGain(antennaGain) { }

    virtual double getValue(const Point<Quaternion>& p) const override {
        return antennaGain->computeGain(std::get<0>(p));
    }

    virtual void partition(const Interval<Quaternion>& i, const std::function<void (const Interval<Quaternion>&, const IFunction<double, Quaternion> *)> f) const override {
        throw cRuntimeError("TODO");
    }

    virtual Interval<double> getRange() const { throw cRuntimeError("TODO"); }
    virtual Interval<Quaternion> getDomain() const { throw cRuntimeError("TODO"); }
    virtual Ptr<const IFunction<double, Quaternion>> limitDomain(const Interval<Quaternion>& i) const { throw cRuntimeError("TODO"); }

    virtual double getMin() const { throw cRuntimeError("TODO"); }
    virtual double getMin(const Interval<Quaternion>& i) const { throw cRuntimeError("TODO"); }

    virtual double getMax() const { throw cRuntimeError("TODO"); }
    virtual double getMax(const Interval<Quaternion>& i) const { throw cRuntimeError("TODO"); }

    virtual double getMean() const { throw cRuntimeError("TODO"); }
    virtual double getMean(const Interval<Quaternion>& i) const { throw cRuntimeError("TODO"); }

    virtual const Ptr<const IFunction<double, Quaternion>> add(const Ptr<const IFunction<double, Quaternion>>& o) const override { throw cRuntimeError("TODO"); }
    virtual const Ptr<const IFunction<double, Quaternion>> subtract(const Ptr<const IFunction<double, Quaternion>>& o) const override { throw cRuntimeError("TODO"); }
    virtual const Ptr<const IFunction<double, Quaternion>> multiply(const Ptr<const IFunction<double, Quaternion>>& o) const override { throw cRuntimeError("TODO"); }
    virtual const Ptr<const IFunction<double, Quaternion>> divide(const Ptr<const IFunction<double, Quaternion>>& o) const override { throw cRuntimeError("TODO"); }
};

} // namespace physicallayer

} // namespace inet

#endif // #ifndef __INET_POWERFUNCTION_H_

