//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NODEBASE_H
#define __INET_NODEBASE_H

#include "inet/node/contract/INetworkNode.h"

namespace inet {

class INET_API NodeBase : public cModule, public INetworkNode
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual INetwork *getNetwork() const override;

    virtual IInterfaceTable *getInterfaceTable() const override;

    virtual IRoutingTable *getRoutingTable() const override;

    virtual void startup() const override;

    virtual void shutdown() const override;

    virtual void crash() const override;
};

} // namespace inet

#endif

