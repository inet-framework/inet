//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INEIGHBORCACHE_H
#define __INET_INEIGHBORCACHE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

/**
 * This interface keeps track of neighbor relationships between radios.
 */
class INET_API INeighborCache : public IPrintableObject
{
  public:
    virtual void addRadio(const IRadio *radio) = 0;

    virtual void removeRadio(const IRadio *radio) = 0;

    /**
     * Sends the provided frame (using the radio medium) to all neighbors within
     * the given range.
     */
    virtual void sendToNeighbors(IRadio *transmitter, const IWirelessSignal *signal, double range) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

