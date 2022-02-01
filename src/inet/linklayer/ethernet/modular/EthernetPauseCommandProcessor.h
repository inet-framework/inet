//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETPAUSECOMMANDPROCESSOR_H
#define __INET_ETHERNETPAUSECOMMANDPROCESSOR_H

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"

namespace inet {

class INET_API EthernetPauseCommandProcessor : public cSimpleModule
{
  protected:
    int seqNum = 0;
    static simsignal_t pauseSentSignal;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  protected:
    void handleSendPause(Request *msg, Ieee802PauseCommand *etherctrl);
};

} // namespace inet

#endif

