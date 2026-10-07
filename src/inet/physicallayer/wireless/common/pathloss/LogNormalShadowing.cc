//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
***************************************************************************/

#include "inet/physicallayer/wireless/common/pathloss/LogNormalShadowing.h"

#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(LogNormalShadowing);

LogNormalShadowing::LogNormalShadowing() :
    sigma(1)
{
}

void LogNormalShadowing::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        sigma = par("sigma");
        correlationDistance = m(par("correlationDistance"));
        if (!std::isnan(correlationDistance.get())) {
            auto medium = check_and_cast<IRadioMedium *>(getParentModule());
            // the medium, whose submodule this is, emits this signal on itself
            check_and_cast<cModule *>(medium)->subscribe(IRadioMedium::radioRemovedSignal, this);
        }
    }
}

std::ostream& LogNormalShadowing::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "LogNormalShadowing";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << EV_FIELD(systemLoss)
               << EV_FIELD(sigma)
               << EV_FIELD(correlationDistance);
    return stream;
}

void LogNormalShadowing::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == IRadioMedium::radioRemovedSignal) {
        int radioId = check_and_cast<IRadio *>(object)->getId();
        int count = 0;
        for (auto it = linkShadowings.begin(); it != linkShadowings.end(); ) {
            if (it->first.first == radioId || it->first.second == radioId) {
                it = linkShadowings.erase(it);
                count++;
            }
            else
                ++it;
        }
        EV_DEBUG << "Dropped the shadowing values of a removed radio" << EV_FIELD(radioId) << EV_FIELD(count) << EV_ENDL;
    }
    else
        throw cRuntimeError("Unknown signal");
}

/**
 * The path loss of one link over frequency, with the shadowing value that was
 * resolved for the link when the function was created.
 */
class LogNormalShadowing::ShadowedPathLossFunction : public math::FunctionBase<double, math::Domain<Hz>>
{
  protected:
    const LogNormalShadowing *pathLoss;
    const mps propagationSpeed;
    const m distance;
    const double shadowing;

  public:
    ShadowedPathLossFunction(const LogNormalShadowing *pathLoss, mps propagationSpeed, m distance, double shadowing) :
        pathLoss(pathLoss), propagationSpeed(propagationSpeed), distance(distance), shadowing(shadowing) {}

    virtual double getValue(const math::Point<Hz>& p) const override {
        return pathLoss->computePathLoss(propagationSpeed, std::get<0>(p), distance, shadowing);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override { os << "(" << *pathLoss << EV_FIELD(distance) << EV_FIELD(shadowing) << ")"; }
};

Ptr<const math::IFunction<double, math::Domain<Hz>>> LogNormalShadowing::computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const Coord& transmitterPosition = transmission->getStartPosition();
    const Coord& receiverPosition = arrival->getStartPosition();
    mps propagationSpeed = transmission->getMedium()->getPropagation()->getPropagationSpeed();
    m distance = m(receiverPosition.distance(transmitterPosition));
    // without a correlation distance: one new value per reception, however often the function is evaluated
    if (std::isnan(correlationDistance.get()))
        return makeShared<ShadowedPathLossFunction>(this, propagationSpeed, distance, normal(0.0, sigma));
    // shadowing is reciprocal: one value per pair of radios, the smaller id first
    int transmitterId = transmission->getTransmitterRadioId();
    int receiverId = receiverRadio->getId();
    bool transmitterFirst = transmitterId < receiverId;
    auto key = transmitterFirst ? std::make_pair(transmitterId, receiverId) : std::make_pair(receiverId, transmitterId);
    const Coord& position1 = transmitterFirst ? transmitterPosition : receiverPosition;
    const Coord& position2 = transmitterFirst ? receiverPosition : transmitterPosition;
    auto it = linkShadowings.find(key);
    if (it == linkShadowings.end()
        || m(it->second.position1.distance(position1)) > correlationDistance
        || m(it->second.position2.distance(position2)) > correlationDistance)
        it = linkShadowings.insert_or_assign(key, LinkShadowing{position1, position2, normal(0.0, sigma)}).first;
    return makeShared<ShadowedPathLossFunction>(this, propagationSpeed, distance, it->second.shadowing);
}

double LogNormalShadowing::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    return computePathLoss(propagationSpeed, frequency, distance, normal(0.0, sigma));
}

double LogNormalShadowing::computePathLoss(mps propagationSpeed, Hz frequency, m distance, double shadowing) const
{
    m d0 = m(1.0);
    // reference path loss
    double freeSpacePathLoss = computeFreeSpacePathLoss(propagationSpeed / frequency, d0, alpha, systemLoss);
    double PL_d0_db = 10.0 * log10(1 / freeSpacePathLoss);
    // path loss at distance d + the shadowing (dB)
    double PL_db = PL_d0_db + 10 * alpha * log10((distance / d0).get<unit>()) + shadowing;
    return math::dB2fraction(-PL_db);
}

} // namespace physicallayer

} // namespace inet

