//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECEPTIONRESULT_H
#define __INET_IRECEPTIONRESULT_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the result of a receiver's reception process.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IReceptionResult : public IPrintableObject
{
  public:
    /**
     * Returns the corresponding reception that also specifies the receiver
     * and the received transmission. This function never returns nullptr.
     */
    virtual const IReception *getReception() const = 0;

    /**
     * Returns the reception decisions made by the receiver in the order of
     * received signal parts. This function never returns an empty vector.
     */
    virtual const std::vector<const IReceptionDecision *> *getDecisions() const = 0;

    /**
     * Returns the packet corresponding to this reception. This function never
     * returns nullptr.
     */
    virtual const Packet *getPacket() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

