//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/common/packet/chunk/FieldsChunk.h"
}}

import inet.networklayer.ted.TED;

namespace inet;

class FieldsChunk extends cObject;

//
// Packet for disseminating link state information (~TELinkStateInfo[]) by the
// ~LinkStateRouting module which implements a minimalistic link state routing
// protocol.
//
class LinkStateMsg extends FieldsChunk
{
    TELinkStateInfo linkInfo[];

    bool request = false; // if true, receiver is expected to send back its own link state database to the sender --FIXME really needed?
        // bool ack = false; -- apparently unused, removed -- TODO check with Vojta

    int command = LINK_STATE_MESSAGE; // FIXME maybe do without this...
}
