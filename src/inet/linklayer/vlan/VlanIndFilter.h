//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANINDFILTER_H
#define __INET_VLANINDFILTER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API VlanIndFilter : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    cValueMap *acceptedVlanIds = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override {}
    virtual void dropPacket(Packet *packet) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

