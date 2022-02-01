//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CARRIERBASEDLIFETIMER_H
#define __INET_CARRIERBASEDLIFETIMER_H

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {

using namespace inet::queueing;

class INET_API CarrierBasedLifeTimer : public cSimpleModule, cListener
{
  protected:
    opp_component_ptr<NetworkInterface> networkInterface;
    ModuleRefByPar<IPacketCollection> packetCollection;

  protected:
    virtual void initialize(int stage) override;
    virtual void clearCollection();

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

