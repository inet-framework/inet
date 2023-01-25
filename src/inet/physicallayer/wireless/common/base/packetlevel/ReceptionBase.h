//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEPTIONBASE_H
#define __INET_RECEPTIONBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

class INET_API ReceptionBase : public virtual IReception, public virtual IReceptionAnalogModel, public cObject
{
  protected:
    const IRadio *receiver;
    const ITransmission *transmission;
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
    ReceptionBase(const IRadio *receiver, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IRadio *getReceiver() const override { return receiver; }
    virtual const ITransmission *getTransmission() const override { return transmission; }

    virtual const simtime_t getStartTime() const override { return startTime; }
    virtual const simtime_t getEndTime() const override { return endTime; }
    virtual const simtime_t getStartTime(IRadioSignal::SignalPart part) const override;
    virtual const simtime_t getEndTime(IRadioSignal::SignalPart part) const override;

    virtual const simtime_t getPreambleStartTime() const override { return startTime; }
    virtual const simtime_t getPreambleEndTime() const override { return startTime + preambleDuration; }
    virtual const simtime_t getHeaderStartTime() const override { return startTime + preambleDuration; }
    virtual const simtime_t getHeaderEndTime() const override { return startTime + preambleDuration + headerDuration; }
    virtual const simtime_t getDataStartTime() const override { return startTime + preambleDuration + headerDuration; }
    virtual const simtime_t getDataEndTime() const override { return startTime + preambleDuration + headerDuration + dataDuration; }

    virtual const simtime_t getDuration() const override { return endTime - startTime; }
    virtual const simtime_t getDuration(IRadioSignal::SignalPart part) const override;

    virtual const simtime_t getPreambleDuration() const override { return preambleDuration; }
    virtual const simtime_t getHeaderDuration() const override { return headerDuration; }
    virtual const simtime_t getDataDuration() const override { return dataDuration; }

    virtual const Coord& getStartPosition() const override { return startPosition; }
    virtual const Coord& getEndPosition() const override { return endPosition; }

    virtual const Quaternion& getStartOrientation() const override { return startOrientation; }
    virtual const Quaternion& getEndOrientation() const override { return endOrientation; }

    virtual const IReceptionAnalogModel *getAnalogModel() const override { return check_and_cast<const IReceptionAnalogModel *>(this); }
};

} // namespace physicallayer

} // namespace inet

#endif

