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

#ifndef __INET_IDEALTRANSMITTER_H
#define __INET_IDEALTRANSMITTER_H

#include "inet/physicallayer/base/packetlevel/TransmitterBase.h"

namespace inet {

namespace physicallayer {

/**
 * Implements the UnitDiskTransmitter model, see the NED file for details.
 */
class INET_API UnitDiskTransmitter : public TransmitterBase
{
  protected:
    simtime_t preambleDuration;
    b headerLength;
    bps bitrate;
    m communicationRange;
    m interferenceRange;
    m detectionRange;

  protected:
    virtual void initialize(int stage) override;

  public:
    UnitDiskTransmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
    virtual simtime_t getPreambleDuration() const { return preambleDuration; }
    virtual b getHeaderLength() const { return headerLength; }
    virtual bps getBitrate() const { return bitrate; }
    virtual m getMaxCommunicationRange() const override { return communicationRange; }
    virtual m getMaxInterferenceRange() const override { return interferenceRange; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IDEALTRANSMITTER_H

