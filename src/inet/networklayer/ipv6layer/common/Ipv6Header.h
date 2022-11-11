//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6HEADER_H
#define __INET_IPV6HEADER_H

#include <list>

#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"

namespace inet {

std::ostream& operator<<(std::ostream& out, const Ipv6ExtensionHeader&);

} // namespace inet

#endif

