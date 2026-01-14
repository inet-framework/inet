//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.packet.chunk.Chunk;
import inet.common.packet.Packet;

namespace inet;

//
// This indication specifies the ICMP error data associated with a transport
// layer error indication. It contains the ICMP packet data that triggered the error.
//
class IcmpErrorInd extends TagBase
{
    // TODO this includes the ICMP header as well.
    // Instead, it should be popped (the checksum verified)
    // and the `type` and `code` fields should be added as
    // separate fields to this tag. See: IcmpHeader chunk.
    Packet *quotedPacket @owned;
}
