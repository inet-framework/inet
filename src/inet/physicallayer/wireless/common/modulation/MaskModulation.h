//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MASKMODULATION_H
#define __INET_MASKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements parameterized amplitude-shift keying that arranges
 * symbols evenly on the x axis.
 *
 * http://en.wikipedia.org/wiki/Amplitude-shift_keying
 */
class INET_API MaskModulation : public ApskModulationBase
{
  public:
    MaskModulation(double maxAmplitude, unsigned int codeWordSize);
    virtual ~MaskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

