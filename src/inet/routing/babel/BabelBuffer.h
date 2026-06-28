//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELBUFFER_H
#define __INET_BABELBUFFER_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/routing/babel/BabelInterfaceTable.h"
#include "inet/routing/babel/BabelMessage_m.h"

namespace inet {
namespace babel {

/**
 * An output buffer that accumulates the TLVs queued for one (destination,
 * interface) pair and flushes them together as a single Babel packet (RFC 6126,
 * section 3.1) once a short buffering deadline elapses. Batching keeps the
 * packet count down and, on a wireless interface, avoids bursting many separate
 * transmissions at the same instant.
 */
class INET_API BabelBuffer : public cObject
{
  protected:
    L3Address dst;
    BabelInterface *outIface = nullptr;
    std::vector<Ptr<BabelTlv>> tlvs;
    BabelTimer *flushTimer = nullptr;
    bool needsAck = false; ///< some buffered TLV asked for reliable (acknowledged) delivery

  public:
    BabelBuffer() {}
    BabelBuffer(const L3Address& da, BabelInterface *oi, BabelTimer *ft) :
        dst(da), outIface(oi), flushTimer(ft)
    {
        ASSERT(oi != nullptr);
        ASSERT(ft != nullptr);
    }

    virtual ~BabelBuffer();
    virtual std::string str() const override;
    friend std::ostream& operator<<(std::ostream& os, const BabelBuffer& b) { return os << b.str(); }

    const L3Address& getDst() const { return dst; }
    BabelInterface *getOutIface() const { return outIface; }
    BabelTimer *getFlushTimer() const { return flushTimer; }

    std::vector<Ptr<BabelTlv>>& getTlvs() { return tlvs; }
    void addTlv(const Ptr<BabelTlv>& tlv) { tlvs.push_back(tlv); }
    bool containsHello() const;

    bool getNeedsAck() const { return needsAck; }
    void setNeedsAck(bool n) { needsAck = n; }

    void clear() { tlvs.clear(); needsAck = false; }
    void deleteFlushTimer();
};

} // namespace babel
} // namespace inet

#endif
