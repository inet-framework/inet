//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMODULATION_H
#define __INET_IMODULATION_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Units.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

/**
 * This interface represents the process of varying one or more physical
 * properties of a periodic waveform, called the carrier signal, with a
 * modulating signal that typically contains information to be transmitted.
 */
class INET_API IModulation : public IPrintableObject, public cObject
{
  public:
    /**
     * Returns the bit error rate as a function of the signal to noise and
     * interference ratio, the bandwidth, and the gross (physical) bitrate.
     */
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const = 0;

    /**
     * Returns the symbol error rate as a function of the signal to noise
     * and interference ratio, the bandwidth, and the gross (physical) bitrate.
     */
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

