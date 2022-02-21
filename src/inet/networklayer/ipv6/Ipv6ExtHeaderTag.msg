//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.ipv6.Ipv6Header;

namespace inet;

cplusplus {{
std::ostream& operator<<(std::ostream& os, Ipv6ExtensionHeader *eh);
}}

class Ipv6ExtHeaderTagBase extends TagBase
{
    Ipv6ExtensionHeader *extensionHeader[] @owned; // array of extension headers, subclassed from ~IPv6ExtensionHeader   //TODO IPv6ExtensionHeadersTag
}

cplusplus(Ipv6ExtHeaderTagBase) {{
  public:
    /**
     * Remove the first extension header and return it.
     */
    Ipv6ExtensionHeader *removeFirstExtensionHeader();

#if (OMNETPP_BUILDNUM < 1530)
    void appendExtensionHeader(Ipv6ExtensionHeader *extensionHeader) { insertExtensionHeader(extensionHeader); }
    Ipv6ExtensionHeader * removeExtensionHeader(size_t k) { return dropExtensionHeader(k); }
#endif
}}

class Ipv6ExtHeaderReq extends Ipv6ExtHeaderTagBase
{
}

class Ipv6ExtHeaderInd extends Ipv6ExtHeaderTagBase
{
}
