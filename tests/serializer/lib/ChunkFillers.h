//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHUNKFILLERS_H
#define __INET_CHUNKFILLERS_H

#include <cstdint>
#include <string>
#include <vector>

#include "ChunkRoundTripTest.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

/**
 * The value source for the chunk fillers: hands out a fresh value per call, built
 * from an ascending byte counter so that
 *
 *   - every byte of a value differs from its neighbours -- a byte-reversed
 *     (wrong-endian) serialization differs from the original,
 *   - consecutive fields of a chunk get different values -- two swapped or
 *     shifted fields do not cancel out,
 *   - no value is 0 or an all-ones sentinel, so a field the serializer forgets
 *     to write comes back different.
 *
 * Deterministic: the same filler always produces the same bytes, so a failure is
 * reproducible from the report alone. Use one instance per fill function:
 *
 *     FillValues v;
 *     p->setSequenceNumber(v.u16());
 *     p->setSourceAddress(v.ipv4());
 */
class FillValues
{
  private:
    unsigned counter;

    /** the next @p n counter bytes as one big-endian value */
    uint64_t nextBytes(int n);

  public:
    explicit FillValues(unsigned seed = 0x11) : counter(seed) {}

    uint8_t u8() { return (uint8_t)nextBytes(1); }
    uint16_t u16() { return (uint16_t)nextBytes(2); }
    uint32_t u32() { return (uint32_t)nextBytes(4); }
    uint64_t u64() { return nextBytes(8); }

    /** an unsigned value that fits a narrow (sub-byte or odd-width) field */
    uint64_t uint(int bits);

    /** a signed value that fits @p bits including the sign bit (positive) */
    int64_t sint(int bits) { return (int64_t)uint(bits - 1); }

    /**
     * A set flag. Always true, because a bool sitting at its `false` default is
     * indistinguishable from one the serializer never writes: the round trip only
     * catches a flag that is written but not read back if it was set in the first
     * place. Where the cleared value matters -- a flag that selects a serializer
     * branch, or two adjacent flags whose swap should be caught -- set it explicitly
     * in the filler, or give the chunk a second variant.
     */
    bool flag() { counter++; return true; }

    /** a small, valid time value that fits the narrow time encodings on the wire */
    simtime_t time() { return SimTime(1 + (counter++ & 7), SIMTIME_MS); }

    /** a short distinct string (NAIs, SSIDs, CNAMEs, ...) */
    std::string text(const char *prefix = "s");

    MacAddress mac();
    Ipv4Address ipv4();
    Ipv6Address ipv6();
    L3Address l3Ipv4() { return L3Address(ipv4()); }
    L3Address l3Ipv6() { return L3Address(ipv6()); }
};

/**
 * All chunk fillers, one entry per (chunk type, variant). Aggregated from the
 * per-protocol groups in lib/fillers/; each group adds its own types, so a new
 * protocol means a new file plus one line here (see ChunkFillers.cc).
 */
std::vector<ChunkFiller> getAllChunkFillers();

// The per-protocol groups, one lib/fillers/ChunkFillers_<group>.cc each. Keep this
// list, the definitions and getAllChunkFillers() in the same order.
void addFillers_common(std::vector<ChunkFiller>& fillers);
void addFillers_ethernet(std::vector<ChunkFiller>& fillers);
void addFillers_linklayer(std::vector<ChunkFiller>& fillers);
void addFillers_ieee80211(std::vector<ChunkFiller>& fillers);
void addFillers_phy(std::vector<ChunkFiller>& fillers);
void addFillers_ipv4(std::vector<ChunkFiller>& fillers);
void addFillers_ipv6(std::vector<ChunkFiller>& fillers);
void addFillers_mipv6(std::vector<ChunkFiller>& fillers);
void addFillers_transport(std::vector<ChunkFiller>& fillers);
void addFillers_sctp(std::vector<ChunkFiller>& fillers);
void addFillers_rtp(std::vector<ChunkFiller>& fillers);
void addFillers_ospf(std::vector<ChunkFiller>& fillers);
void addFillers_routing(std::vector<ChunkFiller>& fillers);
void addFillers_mrp(std::vector<ChunkFiller>& fillers);
void addFillers_gptp(std::vector<ChunkFiller>& fillers);

} // namespace inet

#endif
