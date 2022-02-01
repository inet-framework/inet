//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ILISTENING_H
#define __INET_ILISTENING_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

namespace physicallayer {

class IRadio;

/**
 * This interface represents how a receiver is listening on the radio channel.
 */
class INET_API IListening : public IPrintableObject
{
  public:
    virtual const IRadio *getReceiver() const = 0;

    virtual const simtime_t getStartTime() const = 0;
    virtual const simtime_t getEndTime() const = 0;

    virtual const Coord& getStartPosition() const = 0;
    virtual const Coord& getEndPosition() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

