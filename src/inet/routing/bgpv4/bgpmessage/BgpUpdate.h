//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGPUPDATE_H
#define __INET_BGPUPDATE_H

#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"

namespace inet {
namespace bgp {

INET_API unsigned short computePathAttributeBytes(const BgpUpdatePathAttributes& pathAttr);

} // namespace bgp
} // namespace inet

#endif

