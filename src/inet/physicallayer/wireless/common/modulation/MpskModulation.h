//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MPSKMODULATION_H
#define __INET_MPSKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements parameterized phase-shift keying that arranges
 * symbols evenly on a circle.
 *
 * http://en.wikipedia.org/wiki/Phase-shift_keying#Higher-order_PSK
 */
class INET_API MpskModulation : public ApskModulationBase
{
  public:
    MpskModulation(unsigned int codeWordSize);
    virtual ~MpskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

