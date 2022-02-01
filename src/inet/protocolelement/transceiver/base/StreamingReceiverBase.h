//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMINGRECEIVERBASE_H
#define __INET_STREAMINGRECEIVERBASE_H

#include "inet/protocolelement/transceiver/base/PacketReceiverBase.h"

namespace inet {

class INET_API StreamingReceiverBase : public PacketReceiverBase, public cListener
{
  protected:
    cChannel *transmissionChannel = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

