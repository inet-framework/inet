//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802EPD_H
#define __INET_IEEE802EPD_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"
#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

class INET_API Ieee802Epd : public cSimpleModule, public DefaultProtocolRegistrationListener
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void encapsulate(Packet *frame);
    virtual void decapsulate(Packet *frame);
};

} // namespace inet

#endif

