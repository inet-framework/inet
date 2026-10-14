//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.queueingtest.lib;

import inet.queueing.base.PacketFlowBase;

//
// This module passes packets through without a change, in push and in pull
// mode. It checks the query rules of the queueing contracts on the module that
// it is connected to:
//
// - It asks each query that passes through it numRepeats more times, and it
//   stops the simulation if an answer differs from the first answer.
// - Before each push, it asks the consumer numRepeats more times if it can
//   take the packet.
// - Before each pull, it asks the provider numRepeats more times which packet
//   it offers, and after the pull it stops the simulation if the pulled packet
//   is not the offered packet.
//
// Queries must not change the simulation. So a run with numRepeats = 3 must
// give the same results as a run with numRepeats = 0. With numRepeats = 0 the
// module asks no query of its own and checks nothing.
//
simple QueryProbe extends PacketFlowBase
{
    parameters:
        int numRepeats = default(3); // the number of extra queries for each query and each operation
        @class(QueryProbe);
        @display("i=block/check");
}
