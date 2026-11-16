// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSHANDSHAKEREASSEMBLY_H
#define INET_DTLSHANDSHAKEREASSEMBLY_H

#include <cstdint>
#include <map>
#include <vector>

#include "inet/common/INETDefs.h"

namespace inet::dtls {

/** Bounded incomplete handshake bodies for one association. */
class INET_API DtlsHandshakeReassembly
{
  public:
    enum class Result { EMPTY, OUT_OF_RANGE, CONFLICT, LIMIT, INCOMPLETE, COMPLETE };
    struct Message {
        uint8_t type;
        std::vector<uint8_t> body;
    };

  protected:
    struct Entry {
        uint8_t type;
        uint32_t totalLength;
        std::vector<uint8_t> bytes;
        std::vector<bool> present;
        size_t received = 0;
    };
    size_t limit;
    size_t reservedBytes = 0;
    std::map<uint16_t, Entry> entries;

  public:
    explicit DtlsHandshakeReassembly(size_t limit) : limit(limit) {}

    // Empty fragments never allocate; invalid ranges never mutate retained data.
    Result insert(uint8_t type, uint16_t sequence, uint32_t totalLength, uint32_t offset, const std::vector<uint8_t>& fragment);
    [[nodiscard]] bool isComplete(uint16_t sequence) const;
    [[nodiscard]] bool hasEntry(uint16_t sequence) const { return entries.find(sequence) != entries.end(); }
    [[nodiscard]] size_t firstMissing(uint16_t sequence) const;
    Message take(uint16_t sequence);
    void discard(uint16_t sequence);
    void clear();
    [[nodiscard]] size_t getReservedBytes() const { return reservedBytes; }
    [[nodiscard]] size_t getEntryCount() const { return entries.size(); }
};

} // namespace inet::dtls
#endif
