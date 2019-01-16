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

#include "inet/physicallayer/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/unitdisk/UnitDiskAnalogModel.h"
#include "inet/physicallayer/unitdisk/UnitDiskNoise.h"
#include "inet/physicallayer/unitdisk/UnitDiskReception.h"
#include "inet/physicallayer/unitdisk/UnitDiskSnir.h"
#include "inet/physicallayer/unitdisk/UnitDiskTransmission.h"

namespace inet {
namespace physicallayer {

Define_Module(UnitDiskAnalogModel);

std::ostream& UnitDiskAnalogModel::printToStream(std::ostream& stream, int level) const
{
    return stream << "UnitDiskAnalogModel";
}

const IReception *UnitDiskAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
    const UnitDiskTransmission *idealTransmission = check_and_cast<const UnitDiskTransmission *>(transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const Quaternion receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion receptionEndOrientation = arrival->getEndOrientation();
    m distance = m(transmission->getStartPosition().distance(receptionStartPosition));
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(Hz(NaN), transmission->getStartPosition(), receptionStartPosition) : 1;
    ASSERT(obstacleLoss == 0 || obstacleLoss == 1);
    UnitDiskReception::Power power;
    if (obstacleLoss == 0)
        power = UnitDiskReception::POWER_UNDETECTABLE;
    else if (distance <= idealTransmission->getCommunicationRange())
        power = UnitDiskReception::POWER_RECEIVABLE;
    else if (distance <= idealTransmission->getInterferenceRange())
        power = UnitDiskReception::POWER_INTERFERING;
    else if (distance <= idealTransmission->getDetectionRange())
        power = UnitDiskReception::POWER_DETECTABLE;
    else
        power = UnitDiskReception::POWER_UNDETECTABLE;
    return new UnitDiskReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, power);
}

const INoise *UnitDiskAnalogModel::computeNoise(const IListening *listening, const IInterference *interference) const
{
    bool isInterfering = false;
    for (auto interferingReception : *interference->getInterferingReceptions())
        if (check_and_cast<const UnitDiskReception *>(interferingReception)->getPower() >= UnitDiskReception::POWER_INTERFERING)
            isInterfering = true;
    return new UnitDiskNoise(listening->getStartTime(), listening->getEndTime(), isInterfering);
}

const INoise *UnitDiskAnalogModel::computeNoise(const IReception *reception, const INoise *noise) const
{
    bool isInterfering = check_and_cast<const UnitDiskReception *>(reception)->getPower() >= UnitDiskReception::POWER_INTERFERING;
    return new UnitDiskNoise(reception->getStartTime(), reception->getEndTime(), isInterfering);
}

const ISnir *UnitDiskAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new UnitDiskSnir(reception, noise);
}

} // namespace physicallayer
} // namespace inet

