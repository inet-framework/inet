//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELTOACK_H
#define __INET_BABELTOACK_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/routing/babel/BabelInterfaceTable.h"
#include "inet/routing/babel/BabelMessage_m.h"

namespace inet {
namespace babel {

/**
 * Tracks one message sent reliably (RFC 6126, section 3.1): it carries an
 * Acknowledgment Request with a nonce, and is retransmitted until every
 * intended recipient has acknowledged it or the retransmissions run out.
 */
class INET_API BabelToAck : public cObject
{
  protected:
    uint16_t nonce;
    std::vector<L3Address> dstNodes; ///< recipients that have not acknowledged yet
    int resendNum;
    BabelTimer *resendTimer = nullptr;
    L3Address dst;
    BabelInterface *outIface = nullptr;
    std::vector<Ptr<BabelTlv>> tlvs; ///< the TLVs to retransmit

  public:
    BabelToAck() {}
    BabelToAck(uint16_t n, int rn, BabelTimer *rt, const L3Address& d, BabelInterface *oi, const std::vector<Ptr<BabelTlv>>& m) :
        nonce(n), resendNum(rn), resendTimer(rt), dst(d), outIface(oi), tlvs(m)
    {
        ASSERT(rt != nullptr);
        ASSERT(oi != nullptr);
        resendTimer->setContextPointer(this);
    }

    virtual ~BabelToAck();
    virtual std::string str() const override;
    friend std::ostream& operator<<(std::ostream& os, const BabelToAck& t) { return os << t.str(); }

    uint16_t getNonce() const { return nonce; }
    int decResendNum() { return --resendNum; }

    BabelTimer *getResendTimer() const { return resendTimer; }
    const L3Address& getDst() const { return dst; }
    BabelInterface *getOutIface() const { return outIface; }
    const std::vector<Ptr<BabelTlv>>& getTlvs() const { return tlvs; }

    void addDstNode(const L3Address& dn) { dstNodes.push_back(dn); }
    void removeDstNode(const L3Address& dn);
    size_t dstNodesSize() const { return dstNodes.size(); }

    void resetResendTimer(double delay);
    void deleteResendTimer();
};

} // namespace babel
} // namespace inet

#endif
