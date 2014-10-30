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

#include "inet/physicallayer/base/TransmissionBase.h"

namespace inet {

namespace physicallayer {

class INET_API IdealTransmission : public TransmissionBase
{
  protected:
    const m maxCommunicationRange;
    const m maxInterferenceRange;
    const m maxDetectionRange;

  public:
    IdealTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, m maxCommunicationRange, m maxInterferenceRange, m maxDetectionRange);

    virtual void printToStream(std::ostream& stream) const;

    virtual m getMaxCommunicationRange() const { return maxCommunicationRange; }
    virtual m getMaxInterferenceRange() const { return maxInterferenceRange; }
    virtual m getMaxDetectionRange() const { return maxDetectionRange; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IDEALTRANSMISSION_H

