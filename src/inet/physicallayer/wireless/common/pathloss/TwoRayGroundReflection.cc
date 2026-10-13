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

std::ostream& TwoRayGroundReflection::ReceptionPathLoss::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "TwoRayGroundReflection::ReceptionPathLoss";
    if (level <= PRINT_LEVEL_DEBUG)
        stream << EV_FIELD(distance) << EV_FIELD(transmitterAltitude) << EV_FIELD(receiverAltitude);
    return stream;
}

Ptr<const IReceptionPathLoss> TwoRayGroundReflection::computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    auto radioMedium = transmission->getMedium();
    auto transmitterPosition = transmission->getStartPosition();
    auto recepiverPosition = arrival->getStartPosition();
    mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
    m distance = m(recepiverPosition.distance(transmitterPosition));
    m transmitterAltitude = m(transmitterPosition.distance(physicalEnvironment->getGround()->computeGroundProjection(transmitterPosition)));
    m receiverAltitude = m(recepiverPosition.distance(physicalEnvironment->getGround()->computeGroundProjection(recepiverPosition)));
    return makeShared<ReceptionPathLoss>(this, propagationSpeed, distance, transmitterAltitude, receiverAltitude);
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

