//
// Copyright (C) 2020 OpenSimLtd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETPHY_H
#define __INET_ETHERNETPHY_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace physicallayer {

class INET_API EthernetPhy : public cSimpleModule
{
  protected:
    cGate *physInGate = nullptr;
    cGate *upperLayerInGate = nullptr;

  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace physicallayer

} // namespace inet

#endif

