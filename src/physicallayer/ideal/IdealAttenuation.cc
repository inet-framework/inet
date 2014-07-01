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

#include "IdealAttenuation.h"
#include "IdealTransmission.h"
#include "IdealReception.h"
#include "IArrival.h"
#include "IRadioMedium.h"

namespace inet {

namespace physicallayer {

Define_Module(IdealAttenuation);

const IReception *IdealAttenuation::computeReception(const IRadio *receiverRadio, const ITransmission *transmission) const
{
    const IRadioMedium *channel = receiverRadio->getMedium();
    const IArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const IdealTransmission *idealTransmission = check_and_cast<const IdealTransmission *>(transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    const EulerAngles receptionStartOrientation = arrival->getStartOrientation();
    const EulerAngles receptionEndOrientation = arrival->getEndOrientation();
    m distance = m(transmission->getStartPosition().distance(receptionStartPosition));
    IdealReception::Power power;
    if (distance <= idealTransmission->getMaxCommunicationRange())
        power = IdealReception::POWER_RECEIVABLE;
    else if (distance <= idealTransmission->getMaxInterferenceRange())
        power = IdealReception::POWER_INTERFERING;
    else if (distance <= idealTransmission->getMaxDetectionRange())
        power = IdealReception::POWER_DETECTABLE;
    else
        power = IdealReception::POWER_UNDETECTABLE;
    return new IdealReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, power);
}


}



}
