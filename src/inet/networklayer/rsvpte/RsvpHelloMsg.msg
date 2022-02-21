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
// FIXME all RSVP message have kind=RSVP_TRAFFIC
// to distinguish them:
// if they are RsvpPacket: use packet->getRsvpKind()
// if they are RsvpHelloMsg: only one type exists
//
// thus, we need dynamic_cast to find out what it is
// that's not good. we don't use kind, because kind
// is used by IP QoS mechanism (use DS field for IP QoS instead???)
//
class RsvpHelloMsg extends RsvpMessage
{
    bool request;
    bool ack;

    int srcInstance;
    int dstInstance;

    rsvpKind = HELLO_MESSAGE;
}
