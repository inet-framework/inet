/***************************************************************************
* author:      Andreas Kuntz
*
* copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
*
* author:      Alfonso Ariza
*              Malaga university
*
*              This program is free software; you can redistribute it
*              and/or modify it under the terms of the GNU General Public
*              License as published by the Free Software Foundation; either
*              version 2 of the License, or (at your option) any later
*              version.
*              For further information see file COPYING
*              in the top level directory
***************************************************************************/

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/pathloss/TwoRayGroundReflection.h"

namespace inet {

namespace physicallayer {

Define_Module(TwoRayGroundReflection);

void TwoRayGroundReflection::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        physicalEnvironment = getModuleFromPar<IPhysicalEnvironment>(par("physicalEnvironmentModule"), this, true);
    }
}

std::ostream& TwoRayGroundReflection::printToStream(std::ostream& stream, int level) const
{
    stream << "TwoRayGroundReflection";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", alpha = " << alpha
               << ", systemLoss = " << systemLoss;
    return stream;
}

double TwoRayGroundReflection::computePathLoss(const ITransmission *transmission, const IArrival *arrival) const
{
    auto radioMedium = transmission->getTransmitter()->getMedium();
    auto narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    auto transmitterPosition = transmission->getStartPosition();
    auto recepiverPosition = arrival->getStartPosition();
    mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
    Hz carrierFrequency = narrowbandSignalAnalogModel->getCarrierFrequency();
    m distance = m(recepiverPosition.distance(transmitterPosition));
    m transmitterAltitude = m(transmitterPosition.z - physicalEnvironment->getGround()->getElevation(transmitterPosition));
    m receiverAltitude = m(recepiverPosition.z - physicalEnvironment->getGround()->getElevation(recepiverPosition));
    m waveLength = propagationSpeed / carrierFrequency;
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

