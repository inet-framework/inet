//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFACEOPERATIONS_H
#define __INET_INTERFACEOPERATIONS_H

#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

class NetworkInterface;

/**
 * Base class for lifecycle operations that manipulate a network interface.
 */
class INET_API InterfaceOperationBase : public LifecycleOperation
{
  public:
    enum Stage { STAGE_LOCAL, STAGE_LAST };

  private:
    NetworkInterface *ie; // the interface to be operated on

  public:
    InterfaceOperationBase() : ie(nullptr) {}
    virtual void initialize(cModule *module, StringMap& params);
    virtual int getNumStages() const { return STAGE_LAST + 1; }
    NetworkInterface *getInterface() const { return ie; }
};

/**
 * Lifecycle operation to bring up a network interface.
 */
class INET_API InterfaceUpOperation : public InterfaceOperationBase
{
};

/**
 * Lifecycle operation to bring down a network interface.
 */
class INET_API InterfaceDownOperation : public InterfaceOperationBase
{
};

} // namespace inet

#endif

