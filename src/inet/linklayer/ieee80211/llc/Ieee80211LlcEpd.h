//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211LLCEPD_H
#define __INET_IEEE80211LLCEPD_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"
#include "inet/linklayer/ieee80211/llc/IIeee80211Llc.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211LlcEpd : public cSimpleModule, public IIeee80211Llc
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void encapsulate(Packet *frame);
    virtual void decapsulate(Packet *frame);

  public:
    const Protocol *getProtocol() const override;
};

} // namespace ieee80211
} // namespace inet

#endif

