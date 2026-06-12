//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Tag that carries the index of an IPv6 extension header within the
// IPv6 header's extension header array. Used to pass extension header
// identity from Ipv6 to Mipv6 without using contextPointer.
//
class Ipv6ExtHeaderIndexTag extends TagBase
{
    int extensionHeaderIndex = -1;
}
