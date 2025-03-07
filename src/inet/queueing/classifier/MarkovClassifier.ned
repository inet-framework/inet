//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Stochastic packet classifier that uses a Markov process to determine output gates.
// This module enables probabilistic traffic distribution with time-dependent behavior
// and state transitions.
//
// Operation mechanism:
// - Maintains an internal Markov process with as many states as output gates
// - The current state determines which output gate receives the next packet
// - State transitions occur according to the configured transition probabilities
// - The classifier remains in each state for a configurable amount of time
//
// The MarkovClassifier is particularly useful for:
// - Modeling time-varying traffic patterns
// - Simulating bursty traffic distribution
// - Testing system behavior under probabilistic traffic allocation
// - Implementing load balancing with temporal correlation
// - Modeling channel switching in wireless networks
//
// Key parameters:
// - initialState: The starting state of the Markov process
// - transitionProbabilities: N×N matrix defining state transition probabilities
// - waitIntervals: How long the process stays in each state before transitioning
//
// @see ~PacketClassifier, ~ContentBasedClassifier, ~LabelClassifier
//
simple MarkovClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        displayStringTextFormat = default("classified %p pk (%l)\ncurrent state: %s");
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        int initialState = default(0); // The index of the start state of the Markov process
        string transitionProbabilities; // The transition matrix (N x N) of the Markov process, specified as a list of probabilities
        string waitIntervals; // The amount of time the Markov process stays in a given state, a list of intervals (N) indexed by the state
        @class(MarkovClassifier);
}
