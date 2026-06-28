//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelPenSRTable.h"

#include <sstream>

namespace inet {
namespace babel {

BabelPenSR::~BabelPenSR()
{
    deleteResendTimer();
}

std::string BabelPenSR::str() const
{
    std::stringstream out;
    out << prefix << " orig:" << originator << " reqSN:" << reqSeqno;
    if (receivedFrom != nullptr)
        out << " from:" << receivedFrom->getAddress();
    out << " remains:" << resendNum;
    if (outIface != nullptr)
        out << " on " << outIface->getIfaceName();
    return out.str();
}

void BabelPenSR::resetResendTimer()
{
    uint16_t hinterval = (outIface != nullptr) ? outIface->getHInterval() : defval::HELLO_INTERVAL_CS;
    resetTimer(resendTimer, CStoS(hinterval / 2));
}

void BabelPenSR::resetResendTimer(double delay) { resetTimer(resendTimer, delay); }
void BabelPenSR::deleteResendTimer() { deleteTimer(&resendTimer); }

BabelPenSRTable::~BabelPenSRTable()
{
    removePenSRs();
}

void BabelPenSRTable::removePenSRs()
{
    for (auto req : requests)
        delete req;
    requests.clear();
}

BabelPenSR *BabelPenSRTable::findPenSR(const netPrefix<L3Address>& p)
{
    for (auto req : requests)
        if (req->getPrefix() == p)
            return req;
    return nullptr;
}

BabelPenSR *BabelPenSRTable::findPenSR(const netPrefix<L3Address>& p, BabelInterface *iface)
{
    for (auto req : requests)
        if (req->getPrefix() == p && req->getOutIface() == iface)
            return req;
    return nullptr;
}

BabelPenSR *BabelPenSRTable::addPenSR(BabelPenSR *request)
{
    ASSERT(request != nullptr);
    BabelPenSR *intable = findPenSR(request->getPrefix(), request->getOutIface());
    if (intable != nullptr)
        return intable;
    requests.push_back(request);
    return request;
}

void BabelPenSRTable::removePenSR(BabelPenSR *request)
{
    for (auto it = requests.begin(); it != requests.end(); ++it) {
        if (*it == request) {
            delete *it;
            requests.erase(it);
            return;
        }
    }
}

void BabelPenSRTable::removePenSR(const netPrefix<L3Address>& p)
{
    for (auto it = requests.begin(); it != requests.end();) {
        if ((*it)->getPrefix() == p) {
            delete *it;
            it = requests.erase(it);
        }
        else
            ++it;
    }
}

void BabelPenSRTable::removePenSRsByNeigh(BabelNeighbour *neigh)
{
    for (auto it = requests.begin(); it != requests.end();) {
        if ((*it)->getReceivedFrom() == neigh) {
            delete *it;
            it = requests.erase(it);
        }
        else
            ++it;
    }
}

} // namespace babel
} // namespace inet
