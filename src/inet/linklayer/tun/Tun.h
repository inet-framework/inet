//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// This file is based on the Ppp.h of INET written by Andras Varga.

#ifndef __INET_TUN_H
#define __INET_TUN_H

#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/tun/ITun.h"

namespace inet {

class INET_API Tun : public MacProtocolBase, public ITun, public IModuleInterfaceLookup
{
  protected:
    std::vector<int> socketIds;

  protected:
    virtual void configureNetworkInterface() override;

  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleUpperMessage(cMessage *message) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleUpperCommand(cMessage *message) override;

    virtual void open(int socketId) override;

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;
};

} // namespace inet

#endif

