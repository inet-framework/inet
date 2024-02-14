//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKBASE_H
#define __INET_NETWORKBASE_H

#include "inet/networks/contract/INetwork.h"
#include "inet/node/contract/INetworkNode.h"

namespace inet {

class INET_API NetworkBase : public cModule, public INetwork
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual int getNumNetworkNodes() const override;

    virtual INetworkNode *getNetworkNode(int i) const override;

    virtual INetworkNode *createNetworkNode() const override;

    virtual void addNetworkNode(INetworkNode *networkNode) override;

    virtual void removeNetworkNode(INetworkNode *networkNode) override;
};

} // namespace inet

#endif

