//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.queueing;

import inet.queueing.base.PacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module generates tokens for a packet server based on the observed state
// changes of a packet queue. It's primary purpose is to allow applications to
// produce traffic which completely utilizes a network interface. This is achieved
// by generating tokens whenever the observed packet queue becomes empty.
//
// Available display string text format directives:
//  %p - number of processed packets
//  %l - processed total packet length
//  %t - number of generated tokens
//
simple QueueBasedTokenGenerator
{
    parameters:
        string queueModule; // module path of the observed packet queue
        string serverModule; // module path of the packet server where the tokens are generated via C++ method call
        string displayStringTextFormat = default("generated %t tk"); // determines display string text above the submodule
        volatile double numTokens = default(1); // number of tokens generated when the queue becomes empty
        @class(QueueBasedTokenGenerator);
        @display("i=block/control");
}
