//
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPUDPHOOK_H
#define __INET_SCTPUDPHOOK_H

#include "inet/networklayer/contract/INetfilter.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {

namespace sctp {

class INET_API SctpUdpHook : public cSimpleModule, public NetfilterBase::HookBase
{
  public:
    SctpUdpHook() {}

  public:
    Result datagramPreRoutingHook(Packet *packet) override;
    Result datagramForwardHook(Packet *packet) override { return ACCEPT; }
    Result datagramPostRoutingHook(Packet *packet) override { return ACCEPT; }
    Result datagramLocalInHook(Packet *packet) override { return ACCEPT; }
    Result datagramLocalOutHook(Packet *packet) override { return ACCEPT; }
};

} // namespace sctp

} // namespace inet

#endif

