//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelToAck.h"

#include <sstream>

#include "inet/routing/babel/BabelDefs.h"

namespace inet {
namespace babel {

BabelToAck::~BabelToAck()
{
    deleteResendTimer();
}

void BabelToAck::removeDstNode(const L3Address& dn)
{
    for (auto it = dstNodes.begin(); it != dstNodes.end(); ++it) {
        if (*it == dn) {
            dstNodes.erase(it);
            return;
        }
    }
}

void BabelToAck::resetResendTimer(double delay) { resetTimer(resendTimer, delay); }
void BabelToAck::deleteResendTimer() { deleteTimer(&resendTimer); }

std::string BabelToAck::str() const
{
    std::stringstream out;
    out << "nonce:" << nonce << " to " << dst << " on " << (outIface ? outIface->getIfaceName() : "-")
        << " awaiting:" << dstNodes.size();
    return out.str();
}

} // namespace babel
} // namespace inet
