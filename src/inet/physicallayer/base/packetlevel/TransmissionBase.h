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

#ifndef __INET_TRANSMISSIONBASE_H
#define __INET_TRANSMISSIONBASE_H

#include <memory>

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/packetlevel/ITransmission.h"

namespace inet {
namespace physicallayer {

class IRadioMedium;

class INET_API TransmissionBase : public virtual ITransmission, public virtual ITransmissionAnalogModel, public cObject
{
  protected:
    const int id;
    const IRadioMedium *radioMedium;
    const int transmitterId;
    Ptr<const IAntennaGain> transmitterGain;
    const Packet *packet;
    const simtime_t startTime;
    const simtime_t endTime;
    const simtime_t preambleDuration;
    const simtime_t headerDuration;
    const simtime_t dataDuration;
    const Coord startPosition;
    const Coord endPosition;
    const Quaternion startOrientation;
    const Quaternion endOrientation;

  public:
    TransmissionBase(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation);

    virtual int getId() const override { return id; }

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IRadio *getTransmitter() const override;
    virtual int getTransmitterId() const override { return transmitterId; }
    virtual const IAntennaGain *getTransmitterAntennaGain() const override { return transmitterGain.get(); }
    virtual const IRadioMedium *getMedium() const override { return radioMedium; }
    virtual const Packet *getPacket() const override { return packet; }

    virtual const simtime_t getStartTime() const override { return startTime; }
    virtual const simtime_t getEndTime() const override { return endTime; }
    virtual const simtime_t getStartTime(IRadioSignal::SignalPart part) const override;
    virtual const simtime_t getEndTime(IRadioSignal::SignalPart part) const override;

    virtual const simtime_t getPreambleStartTime() const override { return startTime; }
    virtual const simtime_t getPreambleEndTime() const override { return startTime + preambleDuration; }
    virtual const simtime_t getHeaderStartTime() const override { return startTime + preambleDuration; }
    virtual const simtime_t getHeaderEndTime() const override { return endTime - dataDuration; }
    virtual const simtime_t getDataStartTime() const override { return endTime - dataDuration; }
    virtual const simtime_t getDataEndTime() const override { return endTime; }

    virtual const simtime_t getDuration() const override { return endTime - startTime; }
    virtual const simtime_t getDuration(IRadioSignal::SignalPart part) const override;

    virtual const simtime_t getPreambleDuration() const override { return preambleDuration; }
    virtual const simtime_t getHeaderDuration() const override { return headerDuration; }
    virtual const simtime_t getDataDuration() const override { return dataDuration; }

    virtual const Coord& getStartPosition() const override { return startPosition; }
    virtual const Coord& getEndPosition() const override { return endPosition; }

    virtual const Quaternion& getStartOrientation() const override { return startOrientation; }
    virtual const Quaternion& getEndOrientation() const override { return endOrientation; }

    virtual const ITransmissionAnalogModel *getAnalogModel() const override { return check_and_cast<const ITransmissionAnalogModel *>(this); }
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_TRANSMISSIONBASE_H

