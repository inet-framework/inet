//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETGATE_H
#define __INET_IPACKETGATE_H

#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet gates.
 */
class INET_API IPacketGate
{
  public:
    /**
     * Returns true if the gate is open.
     */
    virtual bool isOpen() const = 0;

    /**
     * Opens the gate and starts traffic go through.
     */
    virtual void open() = 0;

    /**
     * Closes the gate and stops traffic.
     */
    virtual void close() = 0;
};

} // namespace queueing
} // namespace inet

#endif

