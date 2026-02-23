//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv6/Ipv6ExtHeaderTag_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"

namespace inet {

Ipv6ExtensionHeader *Ipv6ExtHeaderTagBase::removeFirstExtensionHeader()
{
    if (extensionHeader_arraysize == 0)
        return nullptr;

    Ipv6ExtensionHeader *ret = removeExtensionHeader(0);
    this->eraseExtensionHeader(0);
    return ret;
}

} // namespace inet

