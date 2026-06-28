//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//
// Protocol-wide constants, enumerations and small value types (router-id,
// route distance, network prefix). These are plain C++ value classes, not
// cObjects; they are used both as fields carried in Babel TLVs and as keys in
// the protocol's databases.
//

#ifndef __INET_BABELDEFS_H
#define __INET_BABELDEFS_H

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {
namespace babel {

namespace defval {
const uint8_t MAGIC = 42;
const uint8_t VERSION = 2;

const int PORT = 6696;
extern const L3Address MCASTG6; // ff02::1:6
extern const L3Address MCASTG4; // 224.0.0.111

// Default intervals (in CENTIseconds!)
const uint16_t HELLO_INTERVAL_CS = 400;
const uint16_t HELLO_INTERVAL_WIRE_CS = 2000;
const uint16_t IHU_INTERVAL_MULT = 3;
const double IHU_HOLD_INTERVAL_MULT = 3.5; ///< IHU hold interval coefficient (multiplied by received IHU interval)
const double ROUTE_EXPIRY_INTERVAL_MULT = 3.5; ///< Route expiry interval coefficient (multiplied by received Update interval)
const uint16_t UPDATE_INTERVAL_MULT = 4; ///< Update interval coefficient (multiplied by hello interval)
const double SOURCE_GC_INTERVAL = 180.0; ///< Source garbage-collection interval

const unsigned int BUFFER_MT_DIVISOR = 4; ///< Maximum-buffer time divisor (hello interval is divided by this number)
const double BUFFER_GC_INTERVAL = 300.0; ///< Buffer garbage-collection interval

const int RESEND_NUM = 3; ///< How many times to try to resend a message to receive an ACK
const uint8_t SEQNUMREQ_HOPCOUNT = 127; ///< Maximal number of forwardings of a SeqnoRequest TLV
const uint16_t NOM_RXCOST_WIRED = 96; ///< Nominal rxcost on wired links
const uint16_t NOM_RXCOST_WIRELESS = 256; ///< Nominal rxcost on wireless links
} // namespace defval

const uint16_t COST_INF = 0xFFFF; ///< Cost infinity
const uint32_t LINK_LOCAL_PREFIX = 0xFE800000; ///< First 32 bits of the IPv6 link-local prefix
const uint16_t HISTORY_LEN = sizeof(uint16_t) * 8; ///< Size of bit vector maintaining the history of received Hello TLVs

const int IPV4_HEADER_SIZE = 20; ///< Size of the IPv4 datagram header
const int IPV6_HEADER_SIZE = 40; ///< Size of the IPv6 datagram header
const int UDP_HEADER_SIZE = 8; ///< Size of the UDP packet header
const int BABEL_HEADER_SIZE = 4; ///< Size of the Babel message header

const int USE_ACK = -1; ///< Send TLV with an ACK request
const double SEND_NOW = 0.0; ///< Send TLV without buffering
const double SEND_URGENT = 0.2; ///< Send TLV with short buffering
const double SEND_BUFFERED = DBL_MAX; ///< Send TLV with buffering

/**
 * Convert centiseconds to seconds.
 */
template<typename T>
inline double CStoS(T cs) { return static_cast<double>(cs) / 100.0; }

/**
 * Number of bytes needed to store plen bits.
 */
template<typename T>
inline int bitsToBytesLen(T plen) { return static_cast<int>(ceil(static_cast<double>(plen) / 8.0)); }

inline bool isLinkLocal64(const Ipv6Address& addr)
{
    return addr.words()[0] == LINK_LOCAL_PREFIX && addr.words()[1] == 0;
}

inline uint16_t plusmod16(uint16_t a, int b) { return (a + b) & 0xFFFF; }
inline int16_t minusmod16(uint16_t a, uint16_t b) { return (a - b) & 0xFFFF; }

/**
 * Modulo-2^16 comparison of sequence numbers.
 * @return  0 if equal, 1 if a > b, and -1 if a < b
 */
inline int comparemod16(uint16_t a, uint16_t b)
{
    if (a == b)
        return 0;
    return ((b - a) & 0x8000) ? 1 : -1;
}

// Address encoding (AE) classification helpers.
int getAeOfAddr(const L3Address& addr);
int getAeOfPrefix(const L3Address& prefix);

/**
 * Types of timer.
 */
struct timerT {
    enum {
        HELLO = 1,
        UPDATE,
        BUFFER,
        BUFFERGC,
        TOACKRESEND,
        NEIGHHELLO,
        NEIGHIHU,
        ROUTEEXPIRY,
        ROUTEBEFEXPIRY,
        SOURCEGC,
        SRRESEND
    };

    static std::string toStr(int timerT);
};

typedef cMessage BabelTimer;

void resetTimer(BabelTimer *timer, double delay);
void deleteTimer(BabelTimer **timer);

/**
 * Address families used on interfaces.
 */
struct AF {
    enum {
        NONE = 0,
        IPvX = 1, ///< Both IPv4 and IPv6
        IPv4 = 4, ///< IPv4 only
        IPv6 = 6 ///< IPv6 only
    };

    static std::string toStr(int af);
};

/**
 * Address encoding.
 */
struct AE {
    enum {
        WILDCARD = 0, ///< Wildcard
        IPv4 = 1, ///< IPv4
        IPv6 = 2, ///< IPv6
        LLIPv6 = 3 ///< Link-local IPv6
    };

    static int maxLen(int ae);
    static int toAF(int ae);
    static std::string toStr(int ae);
};

/**
 * TLV types (RFC 6126, section 4.5).
 */
struct tlvT {
    enum {
        PAD1 = 0,
        PADN = 1,
        ACKREQ = 2,
        ACK = 3,
        HELLO = 4,
        IHU = 5,
        ROUTERID = 6,
        NEXTHOP = 7,
        UPDATE = 8,
        ROUTEREQ = 9,
        SEQNOREQ = 10
    };

    static std::string toStr(int tlvtype);
};

/**
 * 64-bit Babel router-id. Printed as "xxxx:xxxx:xxxx:xxxx".
 */
class rid
{
  protected:
    uint32_t id[2];

    void copy(const rid& other) { id[0] = other.id[0]; id[1] = other.id[1]; }

  public:
    rid() { id[0] = 0; id[1] = 0; }
    rid(uint32_t h, uint32_t l) { id[0] = h; id[1] = l; }
    rid(const Ipv6Address& a) { id[0] = a.words()[2]; id[1] = a.words()[3]; }
    rid(const rid& other) { copy(other); }
    virtual ~rid() {}

    rid& operator=(const rid& other) { if (this == &other) return *this; copy(other); return *this; }

    friend bool operator==(const rid& l, const rid& r) { return l.id[0] == r.id[0] && l.id[1] == r.id[1]; }
    friend bool operator!=(const rid& l, const rid& r) { return !(l == r); }
    friend std::ostream& operator<<(std::ostream& os, const rid& r) { return os << r.str(); }

    std::string str() const;
    const uint32_t *getRid() const { return id; }
    void setRid(uint32_t h, uint32_t l) { id[0] = h; id[1] = l; }
    void setRid(const Ipv6Address& a) { id[0] = a.words()[2]; id[1] = a.words()[3]; }
};

/**
 * A route distance is the pair (seqno, metric). Comparison follows the
 * sequence-number modulo arithmetic of RFC 6126.
 */
class routeDistance
{
  protected:
    uint16_t seqno;
    uint16_t metric;

    void copy(const routeDistance& other) { seqno = other.seqno; metric = other.metric; }

  public:
    routeDistance() : seqno(0), metric(0) {}
    routeDistance(uint16_t s, uint16_t m) : seqno(s), metric(m) {}
    routeDistance(const routeDistance& other) { copy(other); }
    virtual ~routeDistance() {}

    routeDistance& operator=(const routeDistance& other) { if (this == &other) return *this; copy(other); return *this; }

    friend std::ostream& operator<<(std::ostream& os, const routeDistance& dis) { return os << dis.str(); }
    friend bool operator==(const routeDistance& l, const routeDistance& r) { return l.seqno == r.seqno && l.metric == r.metric; }
    friend bool operator!=(const routeDistance& l, const routeDistance& r) { return !(l == r); }
    friend bool operator<(const routeDistance& l, const routeDistance& r)
    {
        return comparemod16(l.seqno, r.seqno) == 1 || (l.seqno == r.seqno && l.metric < r.metric);
    }
    friend bool operator>=(const routeDistance& l, const routeDistance& r) { return !(l < r); }
    friend bool operator>(const routeDistance& l, const routeDistance& r)
    {
        return comparemod16(l.seqno, r.seqno) == -1 || (l.seqno == r.seqno && l.metric > r.metric);
    }
    friend bool operator<=(const routeDistance& l, const routeDistance& r) { return !(l > r); }

    std::string str() const;

    uint16_t getMetric() const { return metric; }
    void setMetric(uint16_t m) { metric = m; }
    uint16_t getSeqno() const { return seqno; }
    void setSeqno(uint16_t s) { seqno = s; }
};

/**
 * A network prefix (address + prefix length). Used as a key throughout the
 * Babel databases. Only the L3Address instantiation is used by the protocol
 * logic; the Ipv4Address/Ipv6Address instantiations exist for completeness.
 *
 * The wire (de)compression helpers (set-from-raw-bytes, copyRaw, bytesToOmit)
 * live with the packet serializer and are added there.
 */
template<typename IPAddress>
class netPrefix
{
  protected:
    IPAddress addr;
    uint8_t len = 0;

    void copy(const netPrefix<IPAddress>& other) { addr = other.addr; len = other.len; }

  public:
    netPrefix(); // specialized per type
    netPrefix(IPAddress a, uint8_t plen) { set(a, plen); }
    netPrefix(const netPrefix<IPAddress>& other) { copy(other); }
    virtual ~netPrefix() {}

    netPrefix<IPAddress>& operator=(const netPrefix<IPAddress>& other) { if (this == &other) return *this; copy(other); return *this; }

    friend std::ostream& operator<<(std::ostream& os, const netPrefix& np) { os << np.str(); return os; }
    friend bool operator==(const netPrefix<IPAddress>& l, const netPrefix<IPAddress>& r) { return l.addr == r.addr && l.len == r.len; }
    friend bool operator!=(const netPrefix<IPAddress>& l, const netPrefix<IPAddress>& r) { return !(l == r); }

    void set(IPAddress a, uint8_t plen); // specialized per type

    std::string str() const
    {
        std::stringstream s;
        s << addr.str() << "/" << static_cast<unsigned int>(len);
        return s.str();
    }

    uint8_t getLen() const { return len; }
    void setLen(uint8_t l) { len = l; }
    const IPAddress& getAddr() const { return addr; }
    void setAddr(IPAddress a) { addr = a; }
    int lenInBytes() const { return bitsToBytesLen(len); }
};

// Explicit specialization declarations (definitions in BabelDefs.cc), so that
// other translation units reference them instead of implicitly instantiating
// the (intentionally undefined) primary template members.
template<> netPrefix<Ipv4Address>::netPrefix();
template<> netPrefix<Ipv6Address>::netPrefix();
template<> netPrefix<L3Address>::netPrefix();
template<> void netPrefix<Ipv4Address>::set(Ipv4Address a, uint8_t plen);
template<> void netPrefix<Ipv6Address>::set(Ipv6Address a, uint8_t plen);
template<> void netPrefix<L3Address>::set(L3Address a, uint8_t plen);

} // namespace babel
} // namespace inet

#endif
