//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IFLATTRANSMISSION_H
#define __INET_IFLATTRANSMISSION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

/**
 * This interface is implemented by the transmissions that consist of a
 * header and a data part, each sent with a single modulation (flat
 * transmissions in the sense of FlatTransmitterBase and FlatReceiverBase),
 * and that know the length and the modulation of the parts even if they have
 * no bit model.
 */
class INET_API IFlatTransmission
{
  public:
    virtual ~IFlatTransmission() {}

    /**
     * Returns the length of the header part in bits, or b(-1) if it is unknown.
     */
    virtual b getHeaderLength() const = 0;

    /**
     * Returns the length of the data part in bits, or b(-1) if it is unknown.
     */
    virtual b getDataLength() const = 0;

    /**
     * Returns the modulation of the header part, or nullptr if it is unknown.
     */
    virtual const IModulation *getHeaderModulation() const = 0;

    /**
     * Returns the modulation of the data part, or nullptr if it is unknown.
     */
    virtual const IModulation *getDataModulation() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif
