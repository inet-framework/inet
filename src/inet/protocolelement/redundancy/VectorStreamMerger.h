//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_VECTORSTREAMMERGER_H
#define __INET_VECTORSTREAMMERGER_H

#include "inet/protocolelement/redundancy/StreamMerger.h"
#include "inet/protocolelement/redundancy/VectorSequenceRecovery.h"

namespace inet {

/** @see VectorStreamMerger.ned */
class INET_API VectorStreamMerger : public StreamMerger
{
  protected:
    static simsignal_t recoveryPassedPacketsSignal;
    static simsignal_t recoveryDiscardedPacketsSignal;
    static simsignal_t recoveryRoguePacketsSignal;
    static simsignal_t recoveryOutOfOrderPacketsSignal;
    static simsignal_t recoveryLostPacketsSignal;
    static simsignal_t recoveryTaglessPacketsSignal;
    static simsignal_t recoveryResetsSignal;

    struct RecoveryState {
        VectorSequenceRecovery recovery;
        cMessage *timer = nullptr;
        explicit RecoveryState(int historyLength) : recovery(historyLength) {}
    };

    int historyLength = VectorSequenceRecovery::DEFAULT_HISTORY_LENGTH;
    simtime_t resetInterval;
    bool takeNoSequence = false;
    bool individualRecovery = false;
    bool initialized = false;
    cValueMap *recoveryMapping = nullptr;
    std::map<std::string, std::unique_ptr<RecoveryState>> recoveryStates;

    int64_t passedPackets = 0;
    int64_t discardedPackets = 0;
    int64_t roguePackets = 0;
    int64_t outOfOrderPackets = 0;
    int64_t lostPackets = 0;
    int64_t taglessPackets = 0;
    int64_t recoveryResets = 0;

    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void handleMessage(cMessage *message) override;
    virtual bool matchesPacket(const Packet *packet) const override;
    virtual void processPacket(Packet *packet) override;
    virtual void handlePacketFiltered(Packet *packet, bool matches) override;
    void configureRecoveryStates();
    const char *getOutputStream(const Packet *packet) const;
    const char *getRecoveryStream(const Packet *packet) const;
    int getSequenceNumber(const Packet *packet) const;
    void reloadTimer(RecoveryState& state);
    void processRecovery(Packet *packet, bool accepted);

  public:
    virtual ~VectorStreamMerger();
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return false; }
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;
};

} // namespace inet

#endif
