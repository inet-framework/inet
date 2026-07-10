//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.networklayer.rsvpte.RsvpPacket;

namespace inet;

//
// RSVP HELLO REQUEST message
//
// This class adds convenience get() and set() methods to the generated
// base class, but no extra data.
//
class RsvpHelloMsg extends RsvpMessage
{
    bool request;
    bool ack;

    int srcInstance;
    int dstInstance;

    rsvpKind = HELLO_MESSAGE;
}
