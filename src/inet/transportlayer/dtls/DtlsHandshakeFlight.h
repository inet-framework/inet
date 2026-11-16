// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSHANDSHAKEFLIGHT_H
#define INET_DTLSHANDSHAKEFLIGHT_H

#include <map>
#include <optional>
#include <set>
#include <vector>

#include "inet/common/packet/chunk/Chunk.h"

namespace inet::dtls {

/** Retained handshake ranges and records actually handed to UDP. */
class INET_API DtlsHandshakeFlight
{
  public:
    struct Fragment {
        uint8_t messageType;
        uint16_t messageSequence;
        uint32_t messageLength;
        uint32_t offset;
        uint64_t epoch;
        Ptr<const Chunk> data;
        bool ready = false;
        bool acknowledged = false;
        bool queued = false;
        bool retransmissionDue = false;
        bool establishAfterSend = false;
        bool serverFinished = false;
        unsigned transmissions = 0;
        bool superseded = false;
        std::vector<size_t> replacements;
        uint8_t recordContentType = 22;
    };
    struct AckResult {
        bool matched = false;
        bool flightComplete = false;
        std::optional<simtime_t> rtt;
    };

  protected:
    struct SentRecord {
        uint16_t messageSequence;
        uint32_t offset;
        uint32_t length;
        simtime_t handoffTime;
        bool retransmitted;
    };
    std::vector<Fragment> fragments;
    std::map<std::pair<uint64_t, uint64_t>, SentRecord> sentRecords;
    std::set<std::pair<uint64_t, uint16_t>> measuredMessages;
    std::map<std::pair<uint64_t, uint16_t>, std::map<uint32_t, uint32_t>> acknowledgedRanges;

  public:
    size_t add(uint8_t messageType, uint16_t messageSequence, uint32_t messageLength, uint32_t offset, uint64_t epoch, const Ptr<const Chunk>& data, bool establishAfterSend = false, bool serverFinished = false, uint8_t recordContentType = 22);
    [[nodiscard]] size_t size() const { return fragments.size(); }
    [[nodiscard]] const Fragment& get(size_t index) const { return fragments.at(index); }
    void markReady(size_t index);
    // Split protected fragments without reusing indices captured by queued operations.
    // Plaintext flights already fit the fixed initial base and are left unchanged.
    bool refragment(size_t maximumBodyBytes);
    [[nodiscard]] bool isFirstTransmissionOrdered(size_t index) const;
    bool reserve(size_t index);
    void cancelReservation(size_t index);
    void requestRetransmission();
    void handedOff(size_t index, uint64_t recordNumber, simtime_t time);
    AckResult acknowledge(uint64_t epoch, uint64_t recordNumber, simtime_t time);
    AckResult acknowledgeSent(simtime_t time);
    [[nodiscard]] bool isComplete() const;
};

} // namespace inet::dtls
#endif
