//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/mpls/MplsPacket_m.h"

namespace inet {

std::string MplsHeader::str() const
{
    std::stringstream out;
    out << "MPLS " << getLabel() << ":" << +getTc() << ":" << +getTtl() << ":" << (getS() ? "Bottom" : "");
    return out.str();
}

} // namespace inet

