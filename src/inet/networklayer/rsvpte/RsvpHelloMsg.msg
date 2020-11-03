//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
// XXX FIXME all RSVP message have kind=RSVP_TRAFFIC
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

