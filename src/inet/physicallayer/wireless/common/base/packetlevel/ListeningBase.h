//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LISTENINGBASE_H
#define __INET_LISTENINGBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"

namespace inet {

namespace physicallayer {

class INET_API ListeningBase : public IListening
{
  protected:
    const IRadio *receiver;
    const simtime_t startTime;
    const simtime_t endTime;
    const Coord startPosition;
    const Coord endPosition;

  public:
    ListeningBase(const IRadio *receiver, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IRadio *getReceiver() const override { return receiver; }

    virtual const simtime_t getStartTime() const override { return startTime; }
    virtual const simtime_t getEndTime() const override { return endTime; }

    virtual const Coord& getStartPosition() const override { return startPosition; }
    virtual const Coord& getEndPosition() const override { return endPosition; }
};

} // namespace physicallayer

} // namespace inet

#endif

