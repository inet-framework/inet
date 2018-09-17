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

#ifndef __INET_IDEALTRANSMISSION_H
#define __INET_IDEALTRANSMISSION_H

#include "inet/physicallayer/base/packetlevel/TransmissionBase.h"

namespace inet {

namespace physicallayer {

/**
 * This model characterizes transmissions with the communication range,
 * interference range, and detection range.
 */
class INET_API UnitDiskTransmission : public TransmissionBase
{
  protected:
    const m communicationRange;
    const m interferenceRange;
    const m detectionRange;

  public:
    UnitDiskTransmission(const IRadio *transmitter, const Packet *macFrame, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, m communicationRange, m interferenceRange, m detectionRange);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual m getCommunicationRange() const { return communicationRange; }
    virtual m getInterferenceRange() const { return interferenceRange; }
    virtual m getDetectionRange() const { return detectionRange; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IDEALTRANSMISSION_H

