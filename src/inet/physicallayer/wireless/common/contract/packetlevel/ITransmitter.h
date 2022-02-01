//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITRANSMITTER_H
#define __INET_ITRANSMITTER_H

#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents a physical device (a part of the radio) which converts
 * packets into electric signals.
 */
class INET_API ITransmitter : public IPrintableObject
{
  public:
    /**
     * Returns the maximum transmission power above which no transmission is
     * ever transmitted. Returns a value in the range [0, +infinity] or NaN
     * if unspecified.
     */
    virtual W getMaxPower() const = 0;

    /**
     * Returns the maximum communication range. Returns a value in the range
     * [0, +infinity] or NaN if unspecified.
     */
    virtual m getMaxCommunicationRange() const = 0;

    /**
     * Returns the maximum interference range. Returns a value in the range
     * [0, +infinity] or NaN if unspecified.
     */
    virtual m getMaxInterferenceRange() const = 0;

    /**
     * Returns a transmission which describes the radio signal corresponding to
     * the provided packet. This function never returns nullptr.
     */
    virtual const ITransmission *createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

