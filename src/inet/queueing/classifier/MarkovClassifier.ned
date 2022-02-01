//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        int initialState = default(0); // the index of the start state of the Markov process
        string transitionProbabilities; // the transition matrix (N x N) of the Markov process, specified as a list of probabilities
        string waitIntervals; // the amount of time the Markov process stays in a given state, a list of intervals (N) indexed by the state
        @class(MarkovClassifier);
}
