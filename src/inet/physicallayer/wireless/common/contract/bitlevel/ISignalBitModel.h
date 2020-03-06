//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISIGNALBITMODEL_H
#define __INET_ISIGNALBITMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/IFecCoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IScrambler.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {
namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the bit domain.
 */
class INET_API ISignalBitModel : public IPrintableObject
{
  public:
    /**
     * Returns the length of the PHY frame header part.
     */
    virtual b getHeaderLength() const = 0;

    /**
     * Returns the length of the PHY frame data part.
     */
    virtual b getDataLength() const = 0;

    /**
     * Returns the gross (physical) bitrate of the PHY frame header part.
     */
    virtual bps getHeaderGrossBitrate() const = 0;

    /**
     * Returns the gross (physical) bitrate of the PHY frame data part.
     */
    virtual bps getDataGrossBitrate() const = 0;

    /**
     * Returns the all bits of the PHY frame header and data parts.
     */
    virtual const BitVector *getAllBits() const = 0;
};

class INET_API ITransmissionBitModel : public virtual ISignalBitModel
{
  public:
    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const = 0;
    virtual const IScrambling *getScrambling() const = 0;
    virtual const IInterleaving *getInterleaving() const = 0;
};

class INET_API IReceptionBitModel : public virtual ISignalBitModel
{
  public:
    virtual double getBitErrorRate() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

