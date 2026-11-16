// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSSESSIONSTORE_H
#define INET_DTLSSESSIONSTORE_H

#include <optional>
#include <string>
#include <vector>

#include "inet/networklayer/common/L3Address.h"

namespace inet::dtls {

/** Bounded engine-local ticket state. A client cache and server table share capacity. */
class INET_API DtlsSessionStore
{
  public:
    struct Entry {
        std::vector<uint8_t> ticket;
        bool serverSide = false;
        L3Address peerAddress;
        int peerPort = -1;
        std::string peerIdentity;
        uint16_t version = 0xfefc;
        uint16_t cipherSuite = 0x1301;
        uint8_t hashBytes = 32;
        simtime_t issuedAt;
        simtime_t expiresAt;
        uint32_t ageAdd = 0;
        uint32_t earlyDataBytes = 0;
        bool earlyConsumed = false;
        uint64_t earlyAssociationId = 0;
        std::vector<uint8_t> earlyClientRandom;
    };

  protected:
    size_t capacity = 0;
    std::vector<Entry> entries;

  public:
    void configure(size_t limit);
    void clear() { entries.clear(); }
    [[nodiscard]] size_t size() const { return entries.size(); }
    bool insert(const Entry& entry, simtime_t now);
    bool consumeEarly(const std::vector<uint8_t>& ticket, const std::vector<uint8_t>& clientRandom, uint64_t associationId, simtime_t now);
    [[nodiscard]] std::optional<Entry> find(const std::vector<uint8_t>& ticket, bool serverSide, simtime_t now) const;
    [[nodiscard]] std::optional<Entry> select(const L3Address& peer, int port, const std::string& identity, uint16_t version, simtime_t now,
                                            const std::vector<uint8_t>& allowedHashes = {}) const;
};

} // namespace inet::dtls
#endif
