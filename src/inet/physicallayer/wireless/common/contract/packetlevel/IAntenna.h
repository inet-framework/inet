//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IANTENNA_H
#define __INET_IANTENNA_H

#include "inet/common/IPrintableObject.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntennaGain.h"

namespace inet {
namespace physicallayer {

/**
 * This interface represents a physical device (a part of the radio) which converts
 * electric signals into radio waves, and vice versa.
 */
class INET_API IAntenna : public IPrintableObject
{
  public:
    /**
     * Returns the mobility of this antenna that describes its position and
     * orientation over time.
     */
    virtual IMobility *getMobility() const = 0;

    /**
     * Returns true if the antenna has directional selectivity (i.e. the gain is not always 1).
     */
    virtual bool isDirectional() const = 0;

    /**
     * Returns the antenna's gain calculator for directional selectivity.
     * This object may be copied as needed, i.e. IAntennaGain objects are
     * expected to have no reference to other objects.
     */
    virtual Ptr<const IAntennaGain> getGain() const = 0;

    /**
     * Returns the number of antennas in the array.
     */
    virtual int getNumAntennas() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

