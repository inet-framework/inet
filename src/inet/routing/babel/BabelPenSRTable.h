//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELPENSRTABLE_H
#define __INET_BABELPENSRTABLE_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/routing/babel/BabelDefs.h"
#include "inet/routing/babel/BabelNeighbourTable.h"

namespace inet {
namespace babel {

/**
 * A pending Seqno Request (RFC 6126, section 3.8.2): a request, sent or
 * forwarded by this node, for a route to a prefix with a sequence number
 * greater than the one currently held. Kept until it is answered or its
 * retransmissions are exhausted.
 */
class INET_API BabelPenSR : public cObject
{
  protected:
    netPrefix<L3Address> prefix;
    rid originator;
    uint16_t reqSeqno = 0;
    uint8_t hopcount = 0;
    BabelNeighbour *receivedFrom = nullptr; ///< neighbour we forward the answer back to (null if our own request)
    int resendNum = 0;
    BabelInterface *outIface = nullptr;
    L3Address forwardTo;
    BabelTimer *resendTimer = nullptr;

  public:
    BabelPenSR() {}
    BabelPenSR(const netPrefix<L3Address>& p, const rid& orig, uint16_t seqno, uint8_t hc,
            BabelNeighbour *recfrom, int resend, BabelInterface *oi, const L3Address& fwddst, BabelTimer *rt) :
        prefix(p), originator(orig), reqSeqno(seqno), hopcount(hc),
        receivedFrom(recfrom), resendNum(resend), outIface(oi), forwardTo(fwddst), resendTimer(rt)
    {
        ASSERT(outIface != nullptr);
        if (resendTimer != nullptr)
            resendTimer->setContextPointer(this);
    }

    virtual ~BabelPenSR();
    virtual std::string str() const override;
    friend std::ostream& operator<<(std::ostream& os, const BabelPenSR& psr) { return os << psr.str(); }

    const netPrefix<L3Address>& getPrefix() const { return prefix; }
    const rid& getOriginator() const { return originator; }
    uint16_t getReqSeqno() const { return reqSeqno; }
    uint8_t getHopcount() const { return hopcount; }

    BabelNeighbour *getReceivedFrom() const { return receivedFrom; }
    void setReceivedFrom(BabelNeighbour *rf) { receivedFrom = rf; }

    int getResendNum() const { return resendNum; }
    int decResendNum() { return --resendNum; }

    BabelInterface *getOutIface() const { return outIface; }
    const L3Address& getForwardTo() const { return forwardTo; }

    BabelTimer *getResendTimer() const { return resendTimer; }
    void setResendTimer(BabelTimer *rt) { resendTimer = rt; }

    void resetResendTimer();
    void resetResendTimer(double delay);
    void deleteResendTimer();
};

class INET_API BabelPenSRTable
{
  protected:
    std::vector<BabelPenSR *> requests;

  public:
    virtual ~BabelPenSRTable();

    std::vector<BabelPenSR *>& getRequests() { return requests; }

    BabelPenSR *findPenSR(const netPrefix<L3Address>& p);
    BabelPenSR *findPenSR(const netPrefix<L3Address>& p, BabelInterface *iface);
    BabelPenSR *addPenSR(BabelPenSR *request);
    void removePenSR(BabelPenSR *request);
    void removePenSR(const netPrefix<L3Address>& p);
    void removePenSRsByNeigh(BabelNeighbour *neigh);
    void removePenSRs();
};

} // namespace babel
} // namespace inet

#endif
