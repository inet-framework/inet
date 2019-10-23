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

package inet.queueing.tokengenerator;

import inet.queueing.base.TokenGeneratorBase;

//
// This module generates tokens for a ~TokenBasedServer based on the observed state
// changes of a packet queue. It's primary purpose is to allow applications to
// produce traffic which completely utilizes a network interface. This is achieved
// by generating tokens whenever the observed packet queue becomes empty.
//
// Available display string text format directives:
//  %p - number of processed packets
//  %l - processed total packet length
//  %t - number of generated tokens
//
simple QueueBasedTokenGenerator extends TokenGeneratorBase
{
    parameters:
        string queueModule; // module path of the observed packet queue
        int minNumPackets = default(1);
        int minTotalLength @unit(b) = default(0 b);
        volatile double numTokens = default(1); // number of tokens generated when the queue becomes empty
        @class(QueueBasedTokenGenerator);
        @display("i=block/control");
        @signal[tokensCreated](type=double);
        @statistic[tokensCreated](title="tokens created"; record=count,sum,vector; interpolationmode=none);
}
