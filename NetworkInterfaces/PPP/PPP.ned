//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//



//
// \PPP implementation.
//
// Packets are encapsulated in PPPFrame.
//
// \PPP is a complex protocol with strong support for link configuration
// and maintenance. This model ignores those details, and only performs
// simple encapsulation/decapsulation and queuing.
//
// In routers, \PPP relies on an external queue module (see OutputQueue)
// to model finite buffer, implement QoS and/or RED, and requests packets
// from this external queue one-by-one.
//
// In hosts, no such queue is used, so \PPP contains an internal
// queue named txQueue to queue up packets waiting for transmission.
// Conceptually, txQueue is of infinite size, but for better diagnostics
// one can specify a hard limit in the txQueueLimit parameter -- if this is
// exceeded, the simulation stops with an error.
//
// There is no buffering done on received packets -- they are just decapsulated
// and sent up immediately.
//
// @see PPPInterface, OutputQueue, PPPFrame
//
simple PPP
    parameters:
        txQueueLimit: numeric,  // only used if queueModule==""; zero means infinite
        queueModule: string;    // name of external (QoS,RED,etc) queue module
    gates:
        in: physIn;
        out: physOut;
        in: netwIn;
        out: netwOut;
endsimple


