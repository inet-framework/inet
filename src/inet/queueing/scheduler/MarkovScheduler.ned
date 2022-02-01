//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// This module implements a packet scheduler using a Markov process that has
// as many states as input gates the scheduler has. The input gate for the next
// packet is determined by the current state of the Markov process.
//
simple MarkovScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        displayStringTextFormat = default("classified %p pk (%l)\ncurrent state: %s");
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        int initialState = default(0); // the start state of the Markov process
        string transitionProbabilities; // the transition matrix (N x N) of the Markov process, specified as a list of probabilities
        string waitIntervals; // the amount of time the Markov process stays in a given state, a list of intervals (N) indexed by the state
        @class(MarkovScheduler);
}
