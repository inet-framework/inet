//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
// The EtherType tags carry the EtherType of a payload whose protocol INET does
// not know, Protocol::unregisteredEthertype. Such a payload has no protocol
// whose EtherType the header inserters could write, so the EtherType travels
// with it: a bridge that forwards such a frame turns the indication of the
// received frame into the request of the frame it sends.
//
class EtherTypeTagBase extends TagBase
{
    int etherType = -1;
}

//
// This request determines the EtherType that the Ethernet MAC header or the
// IEEE 802.1Q tag header inserter writes in front of a payload whose protocol
// is Protocol::unregisteredEthertype.
//
class EtherTypeReq extends EtherTypeTagBase
{
}

//
// This indication specifies the EtherType of a received payload whose protocol
// is Protocol::unregisteredEthertype, as the Ethernet MAC header or the IEEE
// 802.1Q tag header checker found it.
//
class EtherTypeInd extends EtherTypeTagBase
{
}
