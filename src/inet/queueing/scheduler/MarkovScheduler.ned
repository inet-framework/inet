//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// Stochastic packet scheduler that uses a Markov process to determine which
// input gate to pull packets from. This enables modeling complex traffic patterns
// with statistical properties and temporal correlations.
//
// The Markov process has as many states as the scheduler has input gates, with
// each state corresponding to selecting a specific input gate. The scheduler
// transitions between states according to the configured transition probability
// matrix and timing parameters.
//
// Key features:
// - State transitions follow the configured probability matrix
// - Each state has an associated wait interval before transitioning
// - The scheduler selects packets from the input gate corresponding to the current state
// - If the selected input gate has no packets, no packet is scheduled until the next state transition
//
// This scheduler is particularly useful for:
// - Modeling time-varying channel access in wireless networks
// - Simulating bursty traffic patterns with temporal correlation
// - Implementing statistical multiplexing with complex behavior
// - Testing protocol performance under stochastic scheduling conditions
//
// The transitionProbabilities parameter defines an N×N matrix (where N is the number of
// input gates) specifying the probability of transitioning from one state to another.
// The waitIntervals parameter defines how long the scheduler stays in each state.
//
// @see ~PriorityScheduler, ~WrrScheduler
//
simple MarkovScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        displayStringTextFormat = default("classified %p pk (%l)\ncurrent state: %s");
        string clockModule = default(""); // Relative path to a module that implements IClock; optional
        int initialState = default(0); // The starting state of the Markov process
        string transitionProbabilities; // The transition matrix (N x N) of the Markov process, specified as a list of probabilities
        string waitIntervals; // The amount of time the Markov process stays in a given state, a list of intervals (N) indexed by the state
        @class(MarkovScheduler);
}
