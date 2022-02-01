//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETRECEIVER_H
#define __INET_PACKETRECEIVER_H

#include "inet/protocolelement/transceiver/base/PacketReceiverBase.h"

namespace inet {

class INET_API PacketReceiver : public PacketReceiverBase
{
  protected:
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual void receiveSignal(Signal *signal);
};

} // namespace inet

#endif

