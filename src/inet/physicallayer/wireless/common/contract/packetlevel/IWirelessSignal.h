//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IWIRELESSSIGNAL_H
#define __INET_IWIRELESSSIGNAL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IPhysicalLayerFrame.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different signals.
 */
class INET_API IWirelessSignal : public IPhysicalLayerFrame, public IPrintableObject
{
  public:
    /**
     * Returns the radio signal transmission that this signal represents.
     * This function never returns nullptr.
     */
    virtual const ITransmission *getTransmission() const = 0;

    /**
     * This function may return nullptr if this is not yet computed.
     */
    virtual const IArrival *getArrival() const = 0;

    /**
     * This function may return nullptr if this is not yet computed.
     */
    virtual const IListening *getListening() const = 0;

    /**
     * This function may return nullptr if this is not yet computed.
     */
    virtual const IReception *getReception() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

