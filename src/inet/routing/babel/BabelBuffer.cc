//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelBuffer.h"

#include <sstream>

#include "inet/routing/babel/BabelDefs.h"

namespace inet {
namespace babel {

BabelBuffer::~BabelBuffer()
{
    deleteFlushTimer();
}

void BabelBuffer::deleteFlushTimer()
{
    deleteTimer(&flushTimer);
}

bool BabelBuffer::containsHello() const
{
    for (const auto& tlv : tlvs)
        if (tlv->getTlvType() == BABEL_HELLO)
            return true;
    return false;
}

std::string BabelBuffer::str() const
{
    std::stringstream out;
    out << "to " << dst << " on " << (outIface ? outIface->getIfaceName() : "-")
        << ": " << tlvs.size() << " TLV(s)" << (needsAck ? " (ack)" : "");
    return out.str();
}

} // namespace babel
} // namespace inet
