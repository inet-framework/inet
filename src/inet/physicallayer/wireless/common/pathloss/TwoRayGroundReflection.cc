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

double TwoRayGroundReflection::computePathLoss(const ITransmission *transmission, const IArrival *arrival) const
{
    auto radioMedium = transmission->getMedium();
    auto narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    auto transmitterPosition = transmission->getStartPosition();
    auto recepiverPosition = arrival->getStartPosition();
    mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
    Hz centerFrequency = narrowbandSignalAnalogModel->getCenterFrequency();
    m distance = m(recepiverPosition.distance(transmitterPosition));
    m transmitterAltitude = m(transmitterPosition.distance(physicalEnvironment->getGround()->computeGroundProjection(transmitterPosition)));
    m receiverAltitude = m(recepiverPosition.distance(physicalEnvironment->getGround()->computeGroundProjection(recepiverPosition)));
    m waveLength = propagationSpeed / centerFrequency;
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
        return unit((transmitterAltitude * transmitterAltitude * receiverAltitude * receiverAltitude) / (distance * distance * distance * distance * systemLoss)).get();
}

} // namespace physicallayer

} // namespace inet

