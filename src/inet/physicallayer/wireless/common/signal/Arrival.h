//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ARRIVAL_H
#define __INET_ARRIVAL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"

namespace inet {
namespace physicallayer {

class INET_API Arrival : public virtual IArrival
{
  protected:
    const simtime_t startPropagationTime;
    const simtime_t endPropagationTime;
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
    Arrival(const simtime_t startPropagationTime, const simtime_t endPropagationTime, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const simtime_t getStartPropagationTime() const override { return startPropagationTime; }
    virtual const simtime_t getEndPropagationTime() const override { return endPropagationTime; }

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

    virtual const Coord& getStartPosition() const override { return startPosition; }
    virtual const Coord& getEndPosition() const override { return endPosition; }

    virtual const Quaternion& getStartOrientation() const override { return startOrientation; }
    virtual const Quaternion& getEndOrientation() const override { return endOrientation; }
};

} // namespace physicallayer
} // namespace inet

#endif

