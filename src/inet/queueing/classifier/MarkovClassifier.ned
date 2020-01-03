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

package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module implements a packet classifier using a Markov process that has
// as many states as output gates the classifier has. The output gate for
// a given packet is determined by the current state of the Markov process.
//
simple MarkovClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        displayStringTextFormat = default("classified %p pk (%l)\ncurrent state: %s");
        int initialState = default(0); // the index of the start state of the Markov process
        string transitionProbabilities; // the transition matrix (N x N) of the Markov process, specified as a list of probabilities
        string waitIntervals; // the amount of time the Markov process stays in a given state, a list of intervals (N) indexed by the state 
        @class(MarkovClassifier);
}
