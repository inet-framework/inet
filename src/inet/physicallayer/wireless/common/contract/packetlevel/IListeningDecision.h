//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ILISTENINGDECISION_H
#define __INET_ILISTENINGDECISION_H

#include "inet/common/IPrintableObject.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"

namespace inet {
namespace physicallayer {

/**
 * This interface represents the result of a receiver's listening process.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IListeningDecision : public IPrintableObject
{
  public:
    virtual const IListening *getListening() const = 0;

    virtual bool isListeningPossible() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

