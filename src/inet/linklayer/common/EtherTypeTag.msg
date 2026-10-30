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
// not know, Protocol::unknown. The EtherType that the EtherType protocol group
// assigns to Protocol::unknown is INET specific, so the real EtherType travels
// with the payload: a bridge that forwards such a frame turns the indication of
// the received frame into the request of the frame it sends.
//
class EtherTypeTagBase extends TagBase
{
    int etherType = -1;
}

//
// This request determines the EtherType that the Ethernet MAC header or the
// IEEE 802.1Q tag header inserter writes in front of a payload whose protocol
// is Protocol::unknown, in place of the EtherType of Protocol::unknown.
//
class EtherTypeReq extends EtherTypeTagBase
{
}

//
// This indication specifies the EtherType of a received payload whose protocol
// is Protocol::unknown, as the Ethernet MAC header or the IEEE
// 802.1Q tag header checker found it.
//
class EtherTypeInd extends EtherTypeTagBase
{
}
