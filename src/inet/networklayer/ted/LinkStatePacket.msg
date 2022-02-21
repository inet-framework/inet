//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.ted.Ted;

namespace inet;

//
// Packet for disseminating link state information (~TeLinkStateInfo[]) by the
// ~LinkStateRouting module which implements a minimalistic link state routing
// protocol.
//
class LinkStateMsg extends FieldsChunk
{
    TeLinkStateInfo linkInfo[];

    bool request = false; // if true, receiver is expected to send back its own link state database to the sender --FIXME really needed?
        // bool ack = false; -- apparently unused, removed -- TODO check with Vojta

    int command = 1; // FIXME maybe do without this...
}
