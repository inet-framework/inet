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

#include "inet/physicallayer/wireless/common/pathloss/TwoRayGroundReflection.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

using namespace inet::physicalenvironment;

Define_Module(TwoRayGroundReflection);

void TwoRayGroundReflection::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        physicalEnvironment = getModuleFromPar<IPhysicalEnvironment>(par("physicalEnvironmentModule"), this);
    }
}

std::ostream& TwoRayGroundReflection::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "TwoRayGroundReflection";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(alpha)
               << EV_FIELD(systemLoss);
    return stream;
}

/**
 * The path loss of one link over frequency: the link's distance and the
 * altitudes of its two ends are taken when the function is created.
 */
class TwoRayGroundReflection::TwoRayGroundReflectionFunction : public math::FunctionBase<double, math::Domain<Hz>>
{
  protected:
    const TwoRayGroundReflection *pathLoss;
    const mps propagationSpeed;
    const m distance;
    const m transmitterAltitude;
    const m receiverAltitude;

  public:
    TwoRayGroundReflectionFunction(const TwoRayGroundReflection *pathLoss, mps propagationSpeed, m distance, m transmitterAltitude, m receiverAltitude) :
        pathLoss(pathLoss), propagationSpeed(propagationSpeed), distance(distance), transmitterAltitude(transmitterAltitude), receiverAltitude(receiverAltitude) {}

    virtual double getValue(const math::Point<Hz>& p) const override {
        return pathLoss->computeTwoRayGroundReflection(propagationSpeed, std::get<0>(p), distance, transmitterAltitude, receiverAltitude);
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override { os << "(" << *pathLoss << EV_FIELD(distance) << EV_FIELD(transmitterAltitude) << EV_FIELD(receiverAltitude) << ")"; }
};

Ptr<const math::IFunction<double, math::Domain<Hz>>> TwoRayGroundReflection::computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    auto radioMedium = transmission->getMedium();
    auto transmitterPosition = transmission->getStartPosition();
    auto recepiverPosition = arrival->getStartPosition();
    mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
    m distance = m(recepiverPosition.distance(transmitterPosition));
    m transmitterAltitude = m(transmitterPosition.distance(physicalEnvironment->getGround()->computeGroundProjection(transmitterPosition)));
    m receiverAltitude = m(recepiverPosition.distance(physicalEnvironment->getGround()->computeGroundProjection(recepiverPosition)));
    return makeShared<TwoRayGroundReflectionFunction>(this, propagationSpeed, distance, transmitterAltitude, receiverAltitude);
}

double TwoRayGroundReflection::computeTwoRayGroundReflection(mps propagationSpeed, Hz frequency, m distance, m transmitterAltitude, m receiverAltitude) const
{
    m waveLength = propagationSpeed / frequency;
    /**
     * At the cross over distance two ray model and free space model predict the same power
     *
     *                        4 * pi * hr * ht
     *   crossOverDistance = ------------------
     *                           waveLength
     */
    m crossOverDistance = (4 * M_PI * transmitterAltitude * receiverAltitude) / waveLength;
    if (distance < crossOverDistance)
        return computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
    else
        /**
         * Two-ray ground reflection model.
         *
         *         (ht ^ 2 * hr ^ 2)
         * loss = ---------------
         *            d ^ 4 * L
         *
         * To be consistent with the free space equation, L is added here.
         * The original equation in Rappaport's book assumes L = 1.
         */
        return ((transmitterAltitude * transmitterAltitude * receiverAltitude * receiverAltitude) / (distance * distance * distance * distance * systemLoss)).get<unit>();
}

} // namespace physicallayer

} // namespace inet

