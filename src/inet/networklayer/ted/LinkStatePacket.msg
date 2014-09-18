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
#include "inet/networklayer/ted/TED_m.h"
}}

namespace inet;

// Represents a link in the ~TED and in ~LinkStateMsg packets; from TED.msg.
struct TELinkStateInfo;



//
// Packet for disseminating link state information (~TELinkStateInfo[]) by the
// ~LinkStateRouting module which implements a minimalistic link state routing
// protocol.
//
packet LinkStateMsg
{
    TELinkStateInfo linkInfo[];

    bool request = false; // if true, receiver is expected to send back its own link state database to the sender --FIXME really needed?
        // bool ack = false; -- apparently unused, removed -- TODO check with Vojta

    int command = LINK_STATE_MESSAGE; // FIXME maybe do without this...
}
