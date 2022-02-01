//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOUDDELAYERBASE_H
#define __INET_CLOUDDELAYERBASE_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/INetfilter.h"

namespace inet {

// forward declarations:

class INET_API CloudDelayerBase : public cSimpleModule, public NetfilterBase::HookBase
{
  public:
    CloudDelayerBase();
    ~CloudDelayerBase();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;

    /**
     * Returns true in outDrop if the msg is dropped in cloud,
     * otherwise returns calculated delay in outDelay.
     */
    virtual void calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay);

    virtual INetfilter::IHook::Result datagramPreRoutingHook(Packet *datagram) override;
    virtual INetfilter::IHook::Result datagramForwardHook(Packet *datagram) override;
    virtual INetfilter::IHook::Result datagramPostRoutingHook(Packet *datagram) override;
    virtual INetfilter::IHook::Result datagramLocalInHook(Packet *datagram) override;
    virtual INetfilter::IHook::Result datagramLocalOutHook(Packet *datagram) override;

  protected:
    ModuleRefByPar<INetfilter> networkProtocol;
};

} // namespace inet

#endif

