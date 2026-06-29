//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_NETFILTERHOOKPROBE_H
#define __INET_NETFILTERHOOKPROBE_H

#include "inet/common/INETDefs.h"
#include "inet/common/InitStages.h"
#include "inet/networklayer/contract/INetfilter.h"

namespace inet {

/**
 * Test-only netfilter hook that registers on a network-protocol module (Ipv4 or Ipv6)
 * and, for each of the five hooks, returns a configurable verdict (ACCEPT / DROP /
 * QUEUE). On QUEUE it schedules a self-message and later calls reinjectQueuedDatagram(),
 * mimicking how a real hook (IPsec, a reactive routing protocol) defers a datagram.
 *
 * It counts how often each hook fires and how many datagrams it reinjects, so a module
 * test can assert that every hook is invoked and that every QUEUE/reinject path resumes
 * the datagram correctly. Being protocol-agnostic, the same module validates both IPv4
 * and IPv6. See the NED file for parameters.
 */
class INET_API NetfilterHookProbe : public cSimpleModule, public NetfilterBase::HookBase
{
  private:
    INetfilter *netfilter = nullptr;
    simtime_t reinjectDelay;

    IHook::Result preRoutingVerdict = IHook::ACCEPT;
    IHook::Result localInVerdict = IHook::ACCEPT;
    IHook::Result forwardVerdict = IHook::ACCEPT;
    IHook::Result postRoutingVerdict = IHook::ACCEPT;
    IHook::Result localOutVerdict = IHook::ACCEPT;

    long preRoutingCount = 0;
    long localInCount = 0;
    long forwardCount = 0;
    long postRoutingCount = 0;
    long localOutCount = 0;
    long reinjectCount = 0;
    long dropCount = 0;

    static IHook::Result parseVerdict(const char *s);
    IHook::Result process(Packet *datagram, IHook::Result verdict, const char *hookName, long& counter);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

  public:
    virtual IHook::Result datagramPreRoutingHook(Packet *datagram) override;
    virtual IHook::Result datagramForwardHook(Packet *datagram) override;
    virtual IHook::Result datagramPostRoutingHook(Packet *datagram) override;
    virtual IHook::Result datagramLocalInHook(Packet *datagram) override;
    virtual IHook::Result datagramLocalOutHook(Packet *datagram) override;
};

} // namespace inet

#endif
