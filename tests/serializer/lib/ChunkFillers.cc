//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "ChunkFillers.h"

#include <cstdio>

namespace inet {

uint64_t FillValues::nextBytes(int n)
{
    unsigned c = counter;
    counter += 4; // stride > 1, so neighbouring values differ in every byte
    uint64_t v = 0;
    for (int k = 0; k < n; k++)
        v = (v << 8) | ((c + k) & 0xFF);
    return v;
}

uint64_t FillValues::uint(int bits)
{
    if (bits <= 0)
        return 0;
    if (bits > 64)
        bits = 64;
    uint64_t v = nextBytes((bits + 7) / 8);
    if (bits < 64)
        v &= (uint64_t(1) << bits) - 1;
    return v != 0 ? v : 1; // never the "field not written" value
}

std::string FillValues::text(const char *prefix)
{
    return std::string(prefix) + std::to_string(counter++);
}

MacAddress FillValues::mac()
{
    char buf[24];
    unsigned c = counter;
    counter += 6;
    snprintf(buf, sizeof(buf), "%02x-%02x-%02x-%02x-%02x-%02x",
            c & 0xFF, (c + 1) & 0xFF, (c + 2) & 0xFF, (c + 3) & 0xFF, (c + 4) & 0xFF, (c + 5) & 0xFF);
    return MacAddress(buf);
}

Ipv4Address FillValues::ipv4()
{
    unsigned c = counter;
    counter += 4;
    return Ipv4Address(c & 0xFF, (c + 1) & 0xFF, (c + 2) & 0xFF, (c + 3) & 0xFF);
}

Ipv6Address FillValues::ipv6()
{
    char buf[48];
    unsigned c = counter;
    counter += 16;
    snprintf(buf, sizeof(buf), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
            c & 0xFF, (c + 1) & 0xFF, (c + 2) & 0xFF, (c + 3) & 0xFF,
            (c + 4) & 0xFF, (c + 5) & 0xFF, (c + 6) & 0xFF, (c + 7) & 0xFF,
            (c + 8) & 0xFF, (c + 9) & 0xFF, (c + 10) & 0xFF, (c + 11) & 0xFF,
            (c + 12) & 0xFF, (c + 13) & 0xFF, (c + 14) & 0xFF, (c + 15) & 0xFF);
    return Ipv6Address(buf);
}

std::vector<ChunkFiller> getAllChunkFillers()
{
    std::vector<ChunkFiller> fillers;
    addFillers_common(fillers);
    addFillers_ethernet(fillers);
    addFillers_linklayer(fillers);
    addFillers_ieee80211(fillers);
    addFillers_phy(fillers);
    addFillers_ipv4(fillers);
    addFillers_ipv6(fillers);
    addFillers_mipv6(fillers);
    addFillers_transport(fillers);
    addFillers_sctp(fillers);
    addFillers_rtp(fillers);
    addFillers_ospf(fillers);
    addFillers_routing(fillers);
    addFillers_mrp(fillers);
    addFillers_gptp(fillers);
    return fillers;
}

} // namespace inet
