//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TX_H
#define __INET_TX_H

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/ieee80211/mac/contract/ITx.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Mac;
class IRx;

/**
 * The default implementation of ITx.
 */
class INET_API Tx : public SimpleModule, public ITx
{
  protected:
    ITx::ICallback *txCallback = nullptr;
    Ieee80211Mac *mac = nullptr;
    IRx *rx = nullptr;
    Packet *frame = nullptr;
    cMessage *endIfsTimer = nullptr;
    bool transmitting = false;
    TxRequestId requestId;
    uint64_t lifecycleEpoch = 0;
    struct CallbackGuard {
        ITx::ICallback *callback;
        CallbackGuard(ITx::ICallback *callback) : callback(callback) { callback->beginCallback(); }
        ~CallbackGuard() noexcept(false) { callback->endCallback(); }
    };
    void sendPendingFrame();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

  public:
    Tx() {}
    ~Tx();

    void transmitFrame(TxRequestId id, Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, ITx::ICallback *txCallback) override;
    Cancellation cancelPendingTransmission(TxRequestId id) override;
    [[nodiscard]] bool hasTransmission() const override { return txCallback != nullptr; }
    virtual void radioTransmissionFinished() override;
};

} // namespace ieee80211
} // namespace inet

#endif
