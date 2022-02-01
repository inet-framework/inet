//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TX_H
#define __INET_TX_H

#include "inet/linklayer/ieee80211/mac/contract/ITx.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Mac;
class IRx;

/**
 * The default implementation of ITx.
 */
class INET_API Tx : public cSimpleModule, public ITx
{
  protected:
    ITx::ICallback *txCallback = nullptr;
    Ieee80211Mac *mac = nullptr;
    IRx *rx = nullptr;
    Packet *frame = nullptr;
    cMessage *endIfsTimer = nullptr;
    bool transmitting = false;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;

  public:
    Tx() {}
    ~Tx();

    virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, ITx::ICallback *txCallback) override;
    virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, ITx::ICallback *txCallback) override;
    virtual void radioTransmissionFinished() override;
};

} // namespace ieee80211
} // namespace inet

#endif

