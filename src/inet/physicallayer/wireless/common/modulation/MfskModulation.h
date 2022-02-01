//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MFSKMODULATION_H
#define __INET_MFSKMODULATION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements parameterized frequency-shift keying.
 *
 * http://en.wikipedia.org/wiki/Multiple_frequency-shift_keying
 */
class INET_API MfskModulation : public IModulation
{
  protected:
    unsigned int codeWordSize;

  public:
    MfskModulation(unsigned int codeWordSize);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

